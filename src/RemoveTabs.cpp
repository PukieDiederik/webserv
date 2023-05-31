/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RemoveTabs.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gde-alme <gde-alme@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 03:00:03 by gde-alme          #+#    #+#             */
/*   Updated: 2023/05/31 04:03:40 by gde-alme         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RemoveTabs.hpp"
#include <cstdlib>

/* Constructor */
RemoveTabs::RemoveTabs(void) {
}

/* Destructor */
RemoveTabs::~RemoveTabs(void) {
}

std::string removeMultipleSpaces(const std::string& str) {
	std::string result;
	bool previousSpace = false;

	for (std::size_t i = 0; i < str.length(); ++i) {
		if (str[i] != ' ') {
			result += str[i];
			previousSpace = false;
		} else {
			if (!previousSpace) {
				result += str[i];
				previousSpace = true;
			}
		}
	}

	if (!result.empty() && result[0] == ' ')
		result = result.substr(1);

	return result;
}

std::string	RemoveTabs::parseLine(std::string rline, std::string s1, std::string s2) {
	if (rline == "")
		return ("");
	size_t	s1Len = s1.length();
	size_t		pos = 0;
	while (pos < rline.length()) {
		if (rline.compare(pos, s1Len, s1) == 0) {
			rline.erase(pos, s1Len);
			rline.insert(pos, s2);
			pos += s2.length();
		}
		else
			pos++;
	}
	return (removeMultipleSpaces(rline));
}

bool	RemoveTabs::openFile(std::string pathFile) {
	struct stat sb;
	const char *cfile = &pathFile[0];
	if (this->iFileStream.is_open())
		this->iFileStream.close();
	if (pathFile == "" || stat(&pathFile[0], &sb) != 0)
		return (false);
	else {
		this->iFileStream.open(cfile);
		this->fileName = pathFile;
		return (true);
	}
}

bool	RemoveTabs::replace() {
	std::string	s1 = "	", s2 = " ", rline, nline = "";
	while (std::getline(this->iFileStream, rline)) {
		rline = RemoveTabs::parseLine(rline, s1, s2);
		nline += rline;
		nline += "\n";
	}
	this->s3 = nline;
	this->iFileStream.close();
	return (true);
}

void	RemoveTabs::writeToFile(void) {
	std::ofstream	oFileStream;
	std::string		outPathFile;

	outPathFile = ".";
	outPathFile += this->fileName;

	const char	*cco = &outPathFile[0];

	oFileStream.open(cco);
	oFileStream << this->s3;
	oFileStream.close();
}
