#!/bin/bash
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all $*
