#include "../inc/CgiHandler.hpp"
#include "../inc/Request.hpp"
#include "../inc/Location.hpp"
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>

// Forward declarations for utility functions
char fromHexToDec(const std::string& hex);
std::string toString(char c);

/* Constructor */
CgiHandler::CgiHandler() {
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
}

CgiHandler::CgiHandler(std::string path)
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = path;
	this->_ch_env = NULL;
	this->_argv = NULL;
}

CgiHandler::~CgiHandler() {

	if (this->_ch_env)
	{
		for (int i = 0; this->_ch_env[i]; i++)
			free(this->_ch_env[i]);
		free(this->_ch_env);
	}
	if (this->_argv)
	{
		for (int i = 0; this->_argv[i]; i++)
			free(_argv[i]);
		free(_argv);
	}
	this->_env.clear();
}

CgiHandler::CgiHandler(const CgiHandler &other)
{
		this->_env = other._env;
		this->_ch_env = other._ch_env;
		this->_argv = other._argv;
		this->_cgi_path = other._cgi_path;
		this->_cgi_pid = other._cgi_pid;
		this->_exit_status = other._exit_status;
}

CgiHandler &CgiHandler::operator=(const CgiHandler &rhs)
{
    if (this != &rhs)
	{
		this->_env = rhs._env;
		this->_ch_env = rhs._ch_env;
		this->_argv = rhs._argv;
		this->_cgi_path = rhs._cgi_path;
		this->_cgi_pid = rhs._cgi_pid;
		this->_exit_status = rhs._exit_status;
	}
	return (*this);
}

/*Set functions */
void CgiHandler::setCgiPid(pid_t cgi_pid)
{
    this->_cgi_pid = cgi_pid;
}

void CgiHandler::setCgiPath(const std::string &cgi_path)
{
	this->_cgi_path = cgi_path;
}

/* Get functions */
const std::map<std::string, std::string> &CgiHandler::getEnv() const
{
    return (this->_env);
}

const pid_t &CgiHandler::getCgiPid() const
{
    return (this->_cgi_pid);
}

const std::string &CgiHandler::getCgiPath() const
{
    return (this->_cgi_path);
}

void CgiHandler::initEnvCgi(Request& req, const std::vector<Location>::iterator)
{
	// 임시로 기본 CGI 실행파일 경로 사용
	std::string cgi_exec = "cgi-bin/script";
	char    *cwd = getcwd(NULL, 0);
	if(_cgi_path[0] != '/')
	{
		std::string tmp(cwd);
		tmp.append("/");
		if(_cgi_path.length() > 0)
			_cgi_path.insert(0, tmp);
	}
	if(req.getMethod() == POST)
	{
		std::stringstream out;
		out << req.getBody().length();
		this->_env["CONTENT_LENGTH"] = out.str();
		Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Content-Length Passed to cgi is %s", _env["CONTENT_LENGTH"].c_str());
		this->_env["CONTENT_TYPE"] = req.getHeaderValue("content-type");
	}

    this->_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1");
	this->_env["SCRIPT_NAME"] = cgi_exec;//
    this->_env["SCRIPT_FILENAME"] = this->_cgi_path;
    this->_env["PATH_INFO"] = this->_cgi_path;//
    this->_env["PATH_TRANSLATED"] = this->_cgi_path;//
    this->_env["REQUEST_URI"] = this->_cgi_path;//
    this->_env["SERVER_NAME"] = req.getHeaderValue("host");
    this->_env["SERVER_PORT"] ="8002";
    this->_env["REQUEST_METHOD"] = req.getMethodStr();
    this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_env["REDIRECT_STATUS"] = "200";
	this->_env["SERVER_SOFTWARE"] = "AMANIX";

	std::map<std::string, std::string> request_headers = req.getHeaders();
	for(std::map<std::string, std::string>::iterator it = request_headers.begin();
	it != request_headers.end(); ++it)
	{
		std::string name = it->first;
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);
		std::string key = "HTTP_" + name;
		_env[key] = it->second;
	}
	this->_ch_env = (char **)calloc(this->_env.size() + 1, sizeof(char *));
	std::map<std::string, std::string>::const_iterator it = this->_env.begin();
	for (int i = 0; it != this->_env.end(); it++, i++)
	{
		std::string tmp = it->first + "=" + it->second;
		this->_ch_env[i] = strdup(tmp.c_str());
	}
	this->_argv = (char **)malloc(sizeof(char *) * 3);
	this->_argv[0] = strdup(cgi_exec.c_str());
	this->_argv[1] = strdup(this->_cgi_path.c_str());
	this->_argv[2] = NULL;
}


/* initialization environment variable */
// CgiHandler.cpp - initEnv 메서드 수정

void CgiHandler::initEnv(Request& req, const std::vector<Location>::iterator it_loc) {
    int poz;
    std::string extension;
    std::string ext_path;

    extension = this->_cgi_path.substr(this->_cgi_path.find("."));
    
    // 확장자만 추출하도록 수정
    size_t dot_pos = this->_cgi_path.rfind('.');
    if (dot_pos != std::string::npos) {
        extension = this->_cgi_path.substr(dot_pos);
    } else {
        extension = "";
    }
    
    std::cout << "[DEBUG] initEnv() - CGI path: " << this->_cgi_path << std::endl;
    std::cout << "[DEBUG] initEnv() - Extension: " << extension << std::endl;
    
    // Location에서 CGI 경로 가져오기
    if (extension == ".py") {
        ext_path = "/usr/bin/python3";
    } else if (extension == ".sh") {
        ext_path = "/bin/bash";
    } else {
        ext_path = "/usr/bin/php"; // 기본값
    }
    
    std::cout << "[DEBUG] initEnv() - Interpreter path: " << ext_path << std::endl;

    this->_env["AUTH_TYPE"] = "Basic";
    
    // CONTENT_LENGTH와 CONTENT_TYPE 처리 개선
    std::string content_length = req.getHeaderValue("content-length");
    if (content_length.empty()) {
        // GET 요청이나 Content-Length 헤더가 없는 경우 "0"으로 설정
        this->_env["CONTENT_LENGTH"] = "0";
        std::cout << "[DEBUG] CONTENT_LENGTH set to '0' (empty or missing header)" << std::endl;
    } else {
        this->_env["CONTENT_LENGTH"] = content_length;
        std::cout << "[DEBUG] CONTENT_LENGTH set to '" << content_length << "'" << std::endl;
    }
    
    std::string content_type = req.getHeaderValue("content-type");
    if (content_type.empty()) {
        // POST 요청인데 Content-Type이 없는 경우 기본값 설정
        if (req.getMethod() == POST) {
            this->_env["CONTENT_TYPE"] = "application/x-www-form-urlencoded";
            std::cout << "[DEBUG] CONTENT_TYPE set to default for POST" << std::endl;
        } else {
            this->_env["CONTENT_TYPE"] = "";
            std::cout << "[DEBUG] CONTENT_TYPE set to empty (GET request)" << std::endl;
        }
    } else {
        this->_env["CONTENT_TYPE"] = content_type;
        std::cout << "[DEBUG] CONTENT_TYPE set to '" << content_type << "'" << std::endl;
    }
    
    this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
    poz = findStart(this->_cgi_path, "cgi-bin/");
    this->_env["SCRIPT_NAME"] = this->_cgi_path;
    this->_env["SCRIPT_FILENAME"] = ((poz < 0 || (size_t)(poz + 8) > this->_cgi_path.size()) ? "" : this->_cgi_path.substr(poz + 8, this->_cgi_path.size()));
    this->_env["PATH_INFO"] = getPathInfo(req.getPath(), std::vector<std::string>(1, extension));
    this->_env["PATH_TRANSLATED"] = it_loc->getRootPath() + (this->_env["PATH_INFO"] == "" ? "/" : this->_env["PATH_INFO"]);
    this->_env["QUERY_STRING"] = decode(req.getQuery());
    this->_env["REMOTE_ADDR"] = req.getHeaderValue("host");
    poz = findStart(req.getHeaderValue("host"), ":");
    this->_env["SERVER_NAME"] = (poz > 0 ? req.getHeaderValue("host").substr(0, poz) : "");
    this->_env["SERVER_PORT"] = (poz > 0 ? req.getHeaderValue("host").substr(poz + 1, req.getHeaderValue("host").size()) : "");
    this->_env["REQUEST_METHOD"] = req.getMethodStr();
    this->_env["HTTP_COOKIE"] = req.getHeaderValue("cookie");
    this->_env["DOCUMENT_ROOT"] = it_loc->getRootPath();
    this->_env["REQUEST_URI"] = req.getPath() + req.getQuery();
    this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
    this->_env["REDIRECT_STATUS"] = "200";
    this->_env["SERVER_SOFTWARE"] = "AMANIX";

    // 환경 변수 디버깅 출력
    std::cout << "[DEBUG] CGI Environment Variables:" << std::endl;
    std::cout << "[DEBUG]   REQUEST_METHOD=" << this->_env["REQUEST_METHOD"] << std::endl;
    std::cout << "[DEBUG]   CONTENT_LENGTH=" << this->_env["CONTENT_LENGTH"] << std::endl;
    std::cout << "[DEBUG]   CONTENT_TYPE=" << this->_env["CONTENT_TYPE"] << std::endl;
    std::cout << "[DEBUG]   QUERY_STRING=" << this->_env["QUERY_STRING"] << std::endl;

    this->_ch_env = (char **)calloc(this->_env.size() + 1, sizeof(char *));
    std::map<std::string, std::string>::const_iterator it = this->_env.begin();
    for (int i = 0; it != this->_env.end(); it++, i++)
    {
        std::string tmp = it->first + "=" + it->second;
        this->_ch_env[i] = strdup(tmp.c_str());
    }
    this->_argv = (char **)malloc(sizeof(char *) * 3);
    this->_argv[0] = strdup(ext_path.c_str());
    this->_argv[1] = strdup(this->_cgi_path.c_str());
    this->_argv[2] = NULL;
    
    // Debug output
    std::cout << "[DEBUG] initEnv() - Final argv[0]: " << this->_argv[0] << std::endl;
    std::cout << "[DEBUG] initEnv() - Final argv[1]: " << this->_argv[1] << std::endl;
    std::cout << "[DEBUG] initEnv() - Environment size: " << this->_env.size() << std::endl;
}

/* Pipe and execute CGI */
void CgiHandler::execute(short &error_code)
{
	std::cout << "[DEBUG] CGI execute() called with path: " << this->_cgi_path << std::endl;
	
	if (this->_argv[0] == NULL || this->_argv[1] == NULL)
	{
		std::cout << "[DEBUG] CGI execute failed: argv is NULL" << std::endl;
		error_code = 500;
		return ;
	}
	
	std::cout << "[DEBUG] CGI argv[0]: " << this->_argv[0] << std::endl;
	std::cout << "[DEBUG] CGI argv[1]: " << this->_argv[1] << std::endl;
	
	if (pipe(pipe_in) < 0)
	{
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "pipe failed");
		std::cout << "[DEBUG] CGI pipe_in failed" << std::endl;
		error_code = 500;
		return ;
	}
	if (pipe(pipe_out) < 0)
	{
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "pipe failed");
		std::cout << "[DEBUG] CGI pipe_out failed" << std::endl;
		close(pipe_in[0]);
		close(pipe_in[1]);
		error_code = 500;
		return ;
	}
	
	std::cout << "[DEBUG] CGI pipes created successfully" << std::endl;
	
	this->_cgi_pid = fork();
	
	if (this->_cgi_pid == 0)
	{
		// Child process
		std::cerr << "[DEBUG] Child process started, PID: " << getpid() << std::endl;
		std::cerr << "[DEBUG] Child argv[0]: " << this->_argv[0] << std::endl;
		std::cerr << "[DEBUG] Child argv[1]: " << this->_argv[1] << std::endl;
		
		// Print environment variables for debugging
		std::cerr << "[DEBUG] Child environment variables:" << std::endl;
		for (int i = 0; this->_ch_env[i] != NULL; i++) {
			std::cerr << "[DEBUG] env[" << i << "]: " << this->_ch_env[i] << std::endl;
		}
		
		// Set up pipes for stdin and stdout, but keep stderr for error messages
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		
		std::cerr << "[DEBUG] Child about to execve: " << this->_argv[0] << std::endl;
		this->_exit_status = execve(this->_argv[0], this->_argv, this->_ch_env);
		// execve only returns if it fails
		std::cerr << "[DEBUG] Child execve failed: " << strerror(errno) << std::endl;
		std::cerr << "[DEBUG] Child execve errno: " << errno << std::endl;
		exit(127);
	}
	else if (this->_cgi_pid > 0) {
		std::cout << "[DEBUG] CGI parent process, child PID: " << this->_cgi_pid << std::endl;
		
		// Close unused pipe ends in parent
		close(pipe_in[0]);  // Parent doesn't read from stdin pipe
		close(pipe_out[1]); // Parent doesn't write to stdout pipe
		
		std::cout << "[DEBUG] CGI process started successfully" << std::endl;
	}
	else if (this->_cgi_pid > 0){}
	else
	{
        std::cout << "Fork failed" << std::endl;
		error_code = 500;
	}
}

int CgiHandler::findStart(const std::string& path, const std::string& delim)
{
	if (path.empty())
		return (-1);
	size_t poz = path.find(delim);
	if (poz != std::string::npos)
		return (poz);
	else
		return (-1);
}

/* Translation of parameters for QUERY_STRING environment variable */
std::string CgiHandler::decode(const std::string& path)
{
	std::string result = path; // 복사본을 만들어서 수정
	size_t token = result.find("%");
	while (token != std::string::npos)
	{
		if (result.length() < token + 2)
			break ;
		char decimal = fromHexToDec(result.substr(token + 1, 2));
		result.replace(token, 3, HttpUtils::toString(decimal));
		token = result.find("%");
	}
	return (result);
}

/* Isolation PATH_INFO environment variable */
std::string CgiHandler::getPathInfo(const std::string& path, const std::vector<std::string>& extensions)
{
	std::string tmp;
	size_t start, end;

	for (std::vector<std::string>::const_iterator it_ext = extensions.begin(); it_ext != extensions.end(); ++it_ext)
	{
		start = path.find(*it_ext);
		if (start != std::string::npos)
			break ;
	}
	if (start == std::string::npos)
		return "";
	if (start + 3 >= path.size())
		return "";
	tmp = path.substr(start + 3, path.size());
	if (!tmp[0] || tmp[0] != '/')
		return "";
	end = tmp.find("?");
	return (end == std::string::npos ? tmp : tmp.substr(0, end));
}

void		CgiHandler::clear()
{
	this->_cgi_pid = -1;
	this->_exit_status = 0;
	this->_cgi_path = "";
	this->_ch_env = NULL;
	this->_argv = NULL;
	this->_env.clear();
}

// Utility function implementations
char fromHexToDec(const std::string& hex) {
    if (hex.length() != 2) return 0;
    int result = 0;
    for (size_t i = 0; i < 2; ++i) {
        char c = hex[i];
        if (c >= '0' && c <= '9') {
            result = result * 16 + (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            result = result * 16 + (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            result = result * 16 + (c - 'a' + 10);
        }
    }
    return static_cast<char>(result);
}

std::string toString(char c) {
    return std::string(1, c);
}