/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rmvtabs.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gde-alme <gde-alme@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/11 02:58:21 by gde-alme          #+#    #+#             */
/*   Updated: 2023/05/31 03:57:48 by gde-alme         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RemoveTabs.hpp"

int	main(int argc, char **argv) {
	RemoveTabs myLoser;

	std::string	pathToFile = argv[1];

	if (pathToFile.empty()) {
		std::cout << "Error: bad arguments" << std::endl;
		return (2);
	}

	if (!(myLoser.openFile(pathToFile))) {
		std::cout << "Error: no such file or directory" << std::endl;
		return (1);
	}
	myLoser.replace();
	myLoser.writeToFile();
}
