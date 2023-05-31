/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RemoveTabs.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gde-alme <gde-alme@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 03:00:07 by gde-alme          #+#    #+#             */
/*   Updated: 2023/05/31 19:07:35 by gde-alme         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __REMOVETABS_HPP__
#define __REMOVETABS_HPP__

#include <iostream>
#include <fstream>
#include <sys/stat.h>

class RemoveTabs {
	public:
		RemoveTabs(void);
		~RemoveTabs(void);

		bool	openFile(std::string pathFile);
		bool	replace();
		std::string	parseLine(std::string rline, std::string s1, std::string s2);
		void	writeToFile(void);

	private:
		std::string fileName;
		std::string s1;
		std::string	s2;
		std::ifstream iFileStream;
		std::string	s3;
};

#endif /* __REMOVETABS_HPP__ */
