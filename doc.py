#!/bin/python3

import sys
from enum import Enum
import re

class config_field(Enum):
    INPUT = 1
    DOC_TAG = 2

def format_command_line():
    if len(sys.argv) == 2:
        try:
            f = open(sys.argv[1], 'r')
            return f
        except IOError as e:
            print(e)
            sys.exit()
    else:
        print ("format : " + str(sys.argv[0]) + " configfile.")
        sys.exit()


class Config:
    def __init__(self):
        self.input_files = []
        self.doc_tags = []

    def check_param(self):
        if len(self.input_files) == 0:
            print("you need an input")
        if len(self.doc_tags) == 0:
            print("you need an to create a doc tag")


    def print_config(self):
        print(self.input_files)
        print(self.doc_tags)

Conf = Config()


def get_value(line, field):
    global Conf
    wordlist = line.split(' ')
    if field == config_field.INPUT:
        Conf.input_files += wordlist[2:]
    elif field == config_field.DOC_TAG:
        Conf.doc_tags += wordlist[2:]

def get_config(f):
    global Conf
    for l in f:
        if l.split(' ')[:1] == ['INPUT']:
            get_value(l.rstrip(), config_field.INPUT)
        elif l.split(' ')[:1] == ['DOC_TAG']:
            get_value(l.rstrip(), config_field.DOC_TAG)
        elif l == '\n':
            continue
        else :
            print("config file is not well formated :: " + l)
            sys.exit()
    f.close()
    Conf.print_config()
    Conf.check_param()

def strncmp(str1, str2, n):
    if len(str1) < n or len(str2) < n:
        return False
    for i in range(0, n):
        if str1[i] != str2[i]:
            return False
    return True


def remove_space(line):
    for i in range(len(line)):
        if not (line[i] == ' ' or line[i] == '\r' or line[i] == '\t'):
            break
    return line[i:]

def tag_match_line(tags, line):
    match = False
    for tag in tags:
        if strncmp(tag, remove_space(line), len(tag)):
            match = True
            return (True, tag)
    return (False, None)

##[definition][comments]
##format1 : <tag>[section][subsection][...](BLOCK|LINE|FILE|ENDTAG)

class Section

class Comment:
    def __init__(self, line):
        self.line = line

    def get_section:



    def get_lines


def parse_file(f):
    global Conf
    for line in f:
        tag = tag_match_line(Conf.doc_tags, line)
        if tag[0] == False:
            continue
        else:
            print("tag found!")
            Comment comment = Comment(line)




def process_files():
    global Conf
    for f in Conf.input_files:
        print(f)
        try:
            f = open(f, 'r')
            parse_file(f)
        except IOError as e:
            print("Unable to open the file : " + str(f))


if __name__ == '__main__':
    f = format_command_line()
    get_config(f)
    process_files()
