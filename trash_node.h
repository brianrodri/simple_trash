#ifndef TRASH_NODE_H
#define TRASH_NODE_H

#include <unistd.h>
#include <vector>
#include <boost/filesystem.hpp>

#define TRASH_PATH (boost::filesystem::path(getenv("HOME")) / boost::filesystem::path(".trash"))
#define TRASH_INFO_PATH (TRASH_PATH / boost::filesystem::path("info"))

struct trash_node
{
	trash_node(
		size_t in_id,
		boost::filesystem::path in_prevpath,
		boost::filesystem::path in_currpath,
		boost::filesystem::path in_infopath
		) : id(in_id), prevpath(in_prevpath), currpath(in_currpath)
		, infopath(in_infopath) { }
	trash_node(const boost::filesystem::directory_entry& entry);

	// Used for sorting & searching
	bool operator<(const trash_node& rhs) const;
	bool operator<(const size_t& id) const;

	std::string info() const;
	// Both of the following functions leave trash_node in an
	// undefined state
	void remove();
	void restore();

	size_t id;
	boost::filesystem::path prevpath, currpath, infopath;
};

std::vector <trash_node> build_trash_bin();
size_t find_free_id(const std::vector <trash_node>& bin);

#endif
