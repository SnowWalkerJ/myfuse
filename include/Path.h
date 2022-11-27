#ifndef MYFUSE__PATH_H_
#define MYFUSE__PATH_H_
#include <string>
#include <tuple>

std::tuple<std::string, std::string> Split(const std::string &path);

#endif //MYFUSE__PATH_H_
