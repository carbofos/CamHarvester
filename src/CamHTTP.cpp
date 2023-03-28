#include "CamHTTP.h"
#include "CamFunctions.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>

std::pair<std::string, std::string> CamHTTP::parseheader(const std::string_view & source_header_string)
{
    const std::string hdrdelimiter = ":";
    std::string_view hname;
    std::string hvalue;
    auto delimiter_pos = source_header_string.find(hdrdelimiter);
    if (delimiter_pos == std::string::npos)
    {
        throw std::invalid_argument("Can't split header without delimiter (it's normal once per stream): ");
    }    

    hname = source_header_string.substr(0, delimiter_pos);
    hvalue = std::string(source_header_string.substr(delimiter_pos + int(hdrdelimiter.length() ) ));
    if (hvalue.length() > 1 && hvalue.at(0) == ' ' )
        hvalue = hvalue.substr(1, hvalue.length()-1);

    CamFunctions::chop_cr(hvalue);

    std::pair<std::string, std::string> header_pair (hname, hvalue);
    return header_pair;
}

void CamHTTP::add_header(const std::string_view rawheader)
{
    try
    {
        headers.insert( parseheader(rawheader) );
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(debug) << "Add header error: " << e.what() << rawheader ;
    }

}

void CamHTTP::add_multipartheader(const std::string_view rawheader)
{
    try
    {
        headers_multipart.insert( parseheader(rawheader) );
    }
    catch(const std::exception& e)
    {
        BOOST_LOG_TRIVIAL(debug) << "Add multipart header error: " << e.what() << rawheader ;
    }

}

std::string CamHTTP::get_header(const std::string_view name, const std::map<std::string, std::string> &headers_map)
{
for ( const auto &[headername, headervalue] : headers_map)
    if (boost::iequals(headername, name) )
        return headervalue;
throw std::out_of_range("Header not found");
}

size_t CamHTTP::get_content_length(const std::map<std::string, std::string> &headers_map)
{
    std::string contentlength_string;
    try
            {
               return std::stoi( get_header(HTTP_CONTENT_LENGTH, headers_map) );
            }
    catch (std::invalid_argument& )
            {
                std::cerr << "std::invalid_argument in get_content_length: " << contentlength_string << std::endl;
            }
    catch (std::out_of_range& )
            {
                std::cerr << "std::out_of_range in get_content_length: " << contentlength_string << std::endl;
            }
    return 0;
}

int CamHTTP::http_code(const std::string_view &line)
{
    if ( line.substr(0, 5) != "HTTP/")
        return 0;
    std::vector<std::string> http_line_container;
    boost::split(http_line_container, line, boost::is_any_of(" "));
    int http_code = 0;
    try
    {
        http_code = std::stoi( http_line_container[1] );
    }
    catch (...)
    {
        return 0;
    }
    return http_code;
}

bool CamHTTP::isnewline(const std::string_view &l)
{
    if (l == "\n" || l == "" || l == "\r" || l == "\r\n" )  return true;
    return false;
}

std::string CamHTTP::getBoundaryString(const std::string& mime_type_part)
{
    const std::string boundary = "boundary=";
    if (mime_type_part.length() + 2 < HTTP_MIME_TYPE_MULTIPART.length())
        throw std::invalid_argument("Incorrect header length ");

    std::string cleared_boundary_string = mime_type_part.substr(HTTP_MIME_TYPE_MULTIPART.length(),  mime_type_part.length() - HTTP_MIME_TYPE_MULTIPART.length() );

    // cleared_boundary_string.erase(std::remove(cleared_boundary_string.begin(), cleared_boundary_string.end(), ';'), cleared_boundary_string.end());
    cleared_boundary_string.erase(std::remove(cleared_boundary_string.begin(), cleared_boundary_string.end(), ' '), cleared_boundary_string.end());

    auto xparamtokens = CamFunctions::split(cleared_boundary_string, ";");
    for (auto token : xparamtokens)
    {
        auto keyval = CamFunctions::split(token, "=");
        if (keyval.size() > 1)
        {
            if (keyval[0] == "boundary")
                cleared_boundary_string = keyval[1];
            std::cout << keyval[0] << " : " << keyval[1] << std::endl;
        }
    }

    cleared_boundary_string.erase(std::remove(cleared_boundary_string.begin(), cleared_boundary_string.end(), '"'), cleared_boundary_string.end());
    std::cout << cleared_boundary_string<< std::endl;

    CamFunctions::chop_cr(cleared_boundary_string);

    if (! ((cleared_boundary_string.length() > 1) && (cleared_boundary_string.at(0) == '-') && (cleared_boundary_string.at(1) == '-') ) )
    {
        cleared_boundary_string = "--" + cleared_boundary_string;
    }
    return cleared_boundary_string;
}



CamHTTP::states (*CamHTTP::stateprocessors[])(const std::string_view, CamHTTP& http_object) =
{
    [New]                        = CamHTTP::stateprocessor_new, 
    [Start]                      = nullptr,
    [CollectingHeaders]          = stateprocessor_collectingheaders,
    [ProcessHeaders]             = nullptr,
    [CollectingMultipartHeaders] = stateprocessor_collectingmultipartheaders,
    [ProcessMultipartHeaders]    = nullptr,
    [DataByLength]               = stateprocessor_databylength,
    [DataByBoundary]             = stateprocessor_databyboundary,
    [DataByLengthFinish]         = nullptr,
    [DataByBoundaryFinish]       = nullptr,
    [Closing]                    = stateprocessor_closing,
};

const bool CamHTTP::chopping[STATES_LENGTH] =
{
    [New]                        = true,
    [Start]                      = true,
    [CollectingHeaders]          = true,
    [ProcessHeaders]             = true,
    [CollectingMultipartHeaders] = true,
    [ProcessMultipartHeaders]    = true,
    [DataByLength]               = false,
    [DataByBoundary]             = false,
    [DataByLengthFinish]         = true,
    [DataByBoundaryFinish]       = true,
    [Closing]                    = true,
};

CamHTTP::states CamHTTP::stateprocessor_new(const std::string_view line, CamHTTP& http_object)
{
    unused(http_object);
    auto httpcode = http_code(line);
    BOOST_LOG_TRIVIAL(debug) << "HTTP code: " << httpcode;
    if (httpcode == 200 )
        return CamHTTP::states::CollectingHeaders;
    else
        return CamHTTP::states::Closing;
};

CamHTTP::states CamHTTP::stateprocessor_collectingheaders(const std::string_view line, CamHTTP& http_object)
{
    BOOST_LOG_TRIVIAL(debug) << "stateprocessor_collectingheaders add: " << line ;
    if (isnewline( line ))
    {
        BOOST_LOG_TRIVIAL(debug) << "Switching to ProcessHeaders line: " << line ;
        return stateprocessor_processheaders(line, http_object);
    }

    http_object.add_header(line);
    return CamHTTP::states::CollectingHeaders;
}

CamHTTP::states CamHTTP::stateprocessor_processheaders(const std::string_view line, CamHTTP& http_object)
{
    unused(line);
    http_object.headers_multipart.clear();
    std::string contenttype;
    //TODO: image/jpeg support
    try
        {
            contenttype = http_object.get_header(HTTP_CONTENT_TYPE, http_object.headers);
        }
    catch (std::invalid_argument& )
            {
                std::cerr << "std::invalid_argument in stateprocessor_processheaders: " << contenttype << std::endl;
            }
    catch (std::out_of_range& )
            {
                std::cerr << "std::out_of_range in stateprocessor_processheaders: " << contenttype << std::endl;
            }

    if (contenttype.length() > 0 ){
        try
        {
            http_object.boundarystring = getBoundaryString(contenttype);
        }
        catch (std::invalid_argument& )
        {
            std::cerr << "std::invalid_argument in getBoundaryString: " << contenttype << std::endl;
        }

    }
    
    if (http_object.boundarystring.length() > 0 )
            return CamHTTP::states::CollectingMultipartHeaders;

    if (http_object.get_content_length(http_object.headers )  > 0 )
            return CamHTTP::states::DataByLength;

    return CamHTTP::states::DataByBoundary;
}


CamHTTP::states CamHTTP::stateprocessor_collectingmultipartheaders(const std::string_view line, CamHTTP& http_object)
{
    // std::cout << "stateprocessor_collectingmultipartheaders: "  << line << std::endl;
    if (isnewline( line ))
    {
        // std::cout << "Switching to ProcessMultipartHeaders " << line  << std::endl;
        return stateprocessor_processmultipartheaders(line, http_object);
    }

    http_object.add_multipartheader(line);
    return CamHTTP::states::CollectingMultipartHeaders;
}

CamHTTP::states CamHTTP::stateprocessor_processmultipartheaders(const std::string_view line, CamHTTP& http_object)
{
    unused(line);
    std::string contenttype;
    try
        {
            contenttype = http_object.get_header(HTTP_CONTENT_TYPE, http_object.headers_multipart);
        }
    catch (std::invalid_argument& )
            {
                std::cerr << "std::invalid_argument in stateprocessor_processmultipartheaders: " << contenttype << std::endl;
            }
    catch (std::out_of_range& )
            {
                std::cerr << "std::out_of_range in stateprocessor_processmultipartheaders: " << contenttype << std::endl;
            }

    if (contenttype != "image/jpeg" )
    {
        BOOST_LOG_TRIVIAL(debug) << "Only image/jpeg is supported in multipart content< " << contenttype;
        return CamHTTP::states::Closing;
    }

    if (http_object.boundarystring.length() > 0 )
            return CamHTTP::states::DataByBoundary;


    return CamHTTP::states::Closing;
}

CamHTTP::states CamHTTP::stateprocessor_databylength(const std::string_view line, CamHTTP& http_object)
{
    unused(line);
    unused(http_object);
    return CamHTTP::states::Closing;
}

CamHTTP::states CamHTTP::stateprocessor_databyboundary(const std::string_view line, CamHTTP& http_object)
{
    if ( (line.length() >= http_object.boundarystring.length()) && (line.substr(0, http_object.boundarystring.length()) == http_object.boundarystring) )
    {   
        // !!!!!!!!!!!!!!!!!!!!!!!!
        // TODO: Additional 2 bytes CRLF is appented at the end of data
        // BOOST_LOG_TRIVIAL(debug) << " Data received with len " << http_object.data.length()
        // << " content length in headers:  " << http_object.get_content_length(http_object.headers_multipart );
        if (http_object.callback != nullptr)
        {
                if (!(http_object.callback->transmit(std::make_shared<const std::string>( CamHTTP::chopped (http_object.data) ) ) ) )
                    throw std::ios_base::failure("Transmition to customer canceled");
        }
        http_object.data.clear();
        http_object.headers_multipart.clear();
        return CamHTTP::states::CollectingMultipartHeaders;
    }

    http_object.data += line;
    return CamHTTP::states::DataByBoundary;
}

CamHTTP::states CamHTTP::stateprocessor_closing(const std::string_view line, CamHTTP& http_object)
{
    unused(line);
    unused(http_object);
    // std::cout << "stateprocessor_closing"  << std::endl;
    // TODO: table of external callbacks
    return CamHTTP::states::Closing;
}


void CamHTTP::push_newline(const std::string_view &newline)
{
    if (stateprocessors[state_current] == nullptr)
    {
        state_current = CamHTTP::states::Closing;
        throw std::invalid_argument("Missing processor for current state: " + std::to_string(state_current) );
    }
    else
    {
        // std::cout << "Executing processor for state " << state_current  << std::endl;
        state_current = (*stateprocessors[state_current])( chopping[state_current] ? CamHTTP::chopped(newline) : newline , *this);
    }
}

CamHTTP::states CamHTTP::get_state()
{
    return state_current;
}

std::string_view CamHTTP::chopped(const std::string_view line)
{
    if (line.length() < 1)
        return line;

    if (line.substr(line.length()-1, line.length() ) == "\n")
    {
        return line.substr(0, line.length()-1);
    }
    return line;
}

void CamHTTP::set_on_datareceived(CamCallable * callable)
{
    callback = callable;
}
