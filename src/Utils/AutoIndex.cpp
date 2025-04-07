#include "AutoIndex.hpp"

str AutoIndex::getAutoIndex(const str &path)
{
	DIR* dir = opendir(path.c_str());
	if (!dir)
		throw DirectoryNotAccesible();

	struct dirent* entry;
	str	body = AUTOINDEXHEADER;
	str Path = path;
	while ((entry = readdir(dir)) != NULL)
	{
		if ((std::strcmp(entry->d_name, "..") == 0) && path == "/")
			continue;
		else if (std::strcmp(entry->d_name, "..") == 0)
			Path = getPrevPath(path);
		body.append("<li><a href=\"");
		body.append(Path);
		if ((std::strcmp(entry->d_name, ".") == 0) && (std::strcmp(entry->d_name, "..") == 0))
			body.append(entry->d_name);
		body.append("\">");
		body.append(entry->d_name);
		body.append("</a></li>n");
	}
	closedir(dir);
	body.append(AUTOINDEXFOOTER);
	return body;
}

str AutoIndex::getPrevPath(const str &path)
{
	if (path.empty())
		return path;
	std::string ruta = path;
	if (ruta[ruta.size() - 1] == '/')
		ruta.erase(ruta.size() - 1);
	std::size_t pos = ruta.find_last_of('/');
	if (pos == std::string::npos)
		return "/";
	return ruta.substr(0, pos + 1);
}

const char *AutoIndex::DirectoryNotAccesible::what() const throw()
{
	return "No se pudo acceder al directorio.";
}
