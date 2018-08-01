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
    #Conf.print_config()
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
    for tag in tags:
        if strncmp(tag, remove_space(line), len(tag)):
            return (True, tag)
    return (False, None)

##[definition][comments]
##format1 : <tag>[section][subsection][...](BLOCK|LINE|FILE|ENDTAG)




class Section:
    def __init__(self, name):
        self.section_name = name
        self.com_data = []
        self.file_data = []
        self.mode = 0
        self.currently_used = False
        self.sections = []


class Section_Manager():
    def __init__(self):
        self.entry_point_section = Section("ENTRY NODE")
        self.current_section = None

    def add_node(self, parent, sections):
        new_section = Section(sections[0])
        new_section.name = sections[0]
        parent.sections.append(new_section)
        if len(sections[1:]) > 0:
            return self.add_node(new_section, sections[1:])
        else:
            #print("end node")
            new_section.currently_used = True
            self.current_section = new_section

    def create_section(self, entrypoint, sections):
        for s in entrypoint.sections:
            if s.section_name == sections[0]:
                return self.create_section(s, sections[1:])

        self.add_node(entrypoint, sections)

    def add(self, sections, mode):
        sec = self.find(self.entry_point_section, sections)

        #if sec:
        #    print(sec.section_name)

        if self.current_section:
            self.current_section.currently_used = False;

        if sec:
            sec.current_section = True
            self.current_section = sec
        else:
            self.create_section(self.entry_point_section, sections)

    def find(self, entry, sections):
        for n in entry.sections:
            for s in sections:
                if n.section_name == s:
                    if len(sections) == 1:
                        return n
                    return self.find(n, sections[1:])
        return None

    def add_data(self, data):
        sec = self.current_section
        if sec == None:
            print("No current section found data will be add without section")
            #add data
        else:
            sec.com_data.append(data)

    def print_tree(self):
        self.print_node(self.entry_point_section, 0)


    def print_node(self, entry, deepth):
        print(('\t' * deepth) + entry.section_name + "(" + str(int(entry.currently_used)) +  " " + str(len(entry.sections)) + ") " + str(entry.com_data))
        deepth += 1

        for e in entry.sections:
            self.print_node(e, deepth)





sec_mana = Section_Manager()


class Comment:
    def __init__(self, line):
        self.line = line
        self.com_data = ""
        self.parametres = []
        self.mode = []
        self.nb_lines = 0
        self.parse()

    def check_mode(self):
        if self.mode in {'BLOCK', 'LINE', 'FILE', 'ENDTAG'}:
            return True
        return False


    def parse(self):
        #check if line is empty
        if len(self.line) > 0 and self.line[0] == '[':
            val = self.line.split('[', 10)      #get the content within [ ]
            for v in val:
                tmp = (v.split(']', 10))
                for t in tmp:
                    if t:                       #remove all the empty item
                        self.parametres.append(t)
            #print(self.parametres)
            deepth = 0
            sections = []
            for p in self.parametres:
                if p[0] == '(':
                    self.mode = (p[1:].split(')', 1))
                else:
                    sections.append(p)
            sec_mana.add(sections, self.mode)
        else:
            sec_mana.add_data(self.line)


def parse_file(f):
    global Conf
    for line in f:
        tag = tag_match_line(Conf.doc_tags, line)
        if tag[0] == False:
            continue
        else:
            comment = Comment(line.strip()[len(tag[1]):])


def process_files():
    global Conf
    for f in Conf.input_files:
        #print(f)
        try:
            f = open(f, 'r')
            parse_file(f)
        except IOError as e:
            print("Unable to open the file : " + str(f))


if __name__ == '__main__':
    f = format_command_line()
    get_config(f)
    process_files()
    sec_mana.print_tree()
