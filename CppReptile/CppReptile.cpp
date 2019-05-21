#include<string>
#include<iostream>
#include<fstream>
#include<vector>
#include<WinSock2.h>
#include<time.h>
#include<queue>
#include<hash_set>

#pragma comment(lib,"ws2_32.lib")
using namespace std;

const int DEFAULT_PAGE_BUF_SIZE = 1048576; //默认页缓冲区大小？

queue<string> hrefUrl;//url队列
hash_set<string> visitedUrl;//访问过的url
hash_set<string>visitedImg;//访问过的图片

//解析URL
bool ParseURL(const string& url, string& host, string& resourse)
{
	if (strlen(url.c_str())>2000)
	{
		return false;//不解析太长的地址啦
	}

	//判断是否其的子串，考虑到这里只判断了http协议，增加一个判断https的
	const char* pos = strstr(url.c_str(), "http://");
	/*暂时先不改
	if (pos == NULL) {
		pos = strstr(url.c_str(), "https://");
	}*/
	if (pos == NULL) pos = url.c_str;
	else pos += strlen("http://");
	if (strstr(pos, "/") == 0)
		return false;
	char pHost[100];//主机名
	char pResource[2000];//资源名
	sscanf(pos, "%[^/]%s", pHost, pResource);//正则表达式？
	host = pHost;
	resourse = pResource;
	return true;
}

//使用Get请求，得到响应
bool GetHttpResponse(const string& url, char*& response, int& bytesRead)
{
	string host, resource;
	if (!ParseURL(url, host, resource))
	{
		cout << "Can not parse the url" << endl;
		return false;
	}
	//构建socket
	struct hostent* hp = gethostbyname(host.c_str());
	if (hp == NULL)
	{
		cout << "Can not find host address" << endl;
		return false;
	} 

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1 || sock == -2)
	{
		cout << "Can not create socket." << endl;
		return false;
	}

	//建立服务器地址
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(80);//端口
	memcpy(&sa.sin_addr, hp->h_addr_list, 4);
	//建立连接
	if (0 != connect(sock, (SOCKADDR*)& sa, sizeof(sa)))
	{
		cout << "Can not connect: " << url << endl;
		closesocket(sock);
		return false;
	}

	//准备发送数据
	string request = "GET"+resource+"HTTP/1.1\r\nHost:"+host+ "Connection:Close\r\n\r\n";

	//发送数据
	if (SOCKET_ERROR == send(sock, request.c_str(), request.size(), 0))
	{
		cout << "send error" << endl;
		closesocket(sock);
		return false;
	}

	//接受数据
	int m_nContentLength = DEFAULT_PAGE_BUF_SIZE;
	char* pageBuf=(char*)malloc(m_nContentLength);//缓存页

	bytesRead = 0;
	int ret = 1;
	cout << "Read: ";
	while (ret > 0) {
		ret = recv(sock, pageBuf + bytesRead, m_nContentLength - bytesRead, 0);//不断读取
		if (ret > 0)
		{
			bytesRead += ret;//?
		}
		if (m_nContentLength - bytesRead < 100)//空间不够了 增加
		{
			cout << "\nReallock memory" << endl;
			m_nContentLength *= 2;
			pageBuf= (char*)realloc(pageBuf, m_nContentLength);
		}
		cout << ret << " ";
	}
	cout << endl;

	pageBuf[bytesRead] = '\0';
	response = pageBuf;
	closesocket(sock);
	return true;
}

//提取所有的URL以及图片URL
//
void HTMLParse(string& htmlResponse, vector<string>& imgurls, const string& host)
{
	//找到所有连接，加入queue
	const char* p = htmlResponse.c_str;
	const char* tag = "href=\"";//href 属性用于指定超链接目标的 URL
	const char* pos = strstr(p, tag);//在html里找href=
	ofstream ofile("url.txt", ios::app);
	while (pos) {
		pos += strlen(tag);
		const char* nextQ = strstr(pos, "\"");//找"
		if (nextQ)
		{
			char* url = new char[nextQ - pos + 1];
			sscanf(pos, "%[^\"]", url);
		}
	}
}