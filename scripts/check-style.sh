#!/bin/bash

############################################################
# help                                                     #
############################################################
help()
{
   # Display help
   echo "Script to check project style and apply style"
   echo
   echo "Syntax: $0 [-a|c|h]"
   echo "options:"
   echo "-a     Apply style to the project"
   echo "-c     Check whether style is ok without apply it [default]"
   echo "-h     Print this help."
   echo
}

############################################################
# files to check                                              #
############################################################
FILES='include/*.hpp include/**/*.hpp src/*.cpp src/**/*.cpp'


############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# Get the options
while getopts ":ach" option; do
   case $option in
      a) #apply style
        clang-format $FILES -i
        exit;;
      c) # check style
        clang-format $FILES --dry-run --Werror
        exit;;
      h) # display help
         help
         exit;;
      \?)
        echo "Invalid option, please check usage with -h"
        exit;;
   esac
done

clang-format $FILES --dry-run --Werror
