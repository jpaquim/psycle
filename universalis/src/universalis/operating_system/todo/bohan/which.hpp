// -*- mode:c++; indent-tabs-mode:t -*-
#include <dirent>
#include <sys/stat>
#include <string>
bool which(std::string & path, const std::string & name)
{
	DIR * dir = opendir(path.c_str()); if(!dir) return false;
	while(true)
	{
		dirent * entry = readdir(dir);
		if(!entry)
		{
			closedir(dir);
			return false;
		}
		struct stat entry_stat;
		stat((path + entry->d_name).c_str(), &entry_stat);
		if(S_ISDIR(entry_stat.st_mode))
		{
			std::string deeper_path(path);
			deeper_path += '\\';
			deeper_path += entry->d_name;
			if(which(deeper_path, name))
			{
				closedir(dir);
				path = deeper_path;
				return true;
			}
		}
		else if(name == entry->d_name)
		{
			closedir(dir);
			return true;
		}
	}
}

// arch-tag: 8b0b66d9-fb7c-41c4-a6dd-e375c2bd465e
