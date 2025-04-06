#pragma	once
#include "../WebSrv.hpp"
#include <iostream>
#include <dirent.h>
#define AUTOINDEXHEADER "<!DOCTYPE html>\n<html lang=\"es\">\n<head><title>AutoIndex</title></head>\n<body>\n<ul>\n"
#define AUTOINDEXFOOTER "</ul></body></html>"
class AutoIndex
{
	public:
		static str getAutoIndex(const str &path);
		static str getPrevPath(const str & path);

		class DirectoryNotAccesible : public std::exception {
			public:
				virtual const char* what() const throw();
			};
	private:
	AutoIndex();
	~AutoIndex();

};

