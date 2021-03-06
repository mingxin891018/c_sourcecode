#!/bin/sh
BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
YELLOW=$(tput setaf 3)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
NORMAL=$(tput sgr0)

###
### Funcitons Define
###
function red()
{
	echo -e "${RED} ${1} ${NORMAL}"
}
function green()
{
	echo -e "${GREEN} ${1} ${NORMAL}"
}
function yellow()
{
	echo -e "${YELLOW} ${1} ${NORMAL}"
}
function blue()
{
	echo -e "${BLUE} ${1} ${NORMAL}"
}
function magenta()
{
	echo -e "${MAGENTA} ${1} ${NORMAL}"
}
function cyan()
{
	echo -e "${CYAN} ${1} ${NORMAL}"
}
function help()
{
	cat <<XXOO
		Usage: ${0} ${1}
XXOO
}

###
### Main Logic
###
blue "blue font"
red "red font"
green "green font"
yellow "yellow font"
magenta "magenta font"
cyan "cyan font"
help
