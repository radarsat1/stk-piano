#!/usr/bin/env python

import sys

# Convert a text file to a Prosper Latex file to generate slides

class txt2prosper:
	def __init__(self):
		pass

	def convert(self, infile, outfile):
		self.indent = []
		self.inslide = False
		self.infile = infile
		self.outfile = outfile
		self.newParagraph = False
		self.preformat = False
		for line in infile:
			self.parseLine(line)
		if self.inslide:
			outfile.write('\\end{slide}\n')

	def setLevel(self, level):
		if level > len(self.indent):
			level = len(self.indent)
		for i in range(len(self.indent)-level):
			self.outfile.write('  '*len(self.indent)+'\\end{itemize}\n')
			self.indent = self.indent[:-1]

	def addLevel(self, whitespace):
		self.indent.append(whitespace)

	def parseLine(self, line):
		if self.preformat:
			if len(line)>0 and line[0]=='@':
				self.preformat = False
				self.outfile.write('\n')
			else:
				self.outfile.write(line)
			return
		
		whitespace = 0
		for c in line:
			if c==' ':
				whitespace+=1
			elif c=='\t':
				whitespace+=4
			else:
				break

		line = line.strip()
		if len(line)>0:
			c = line[0]
			
			if c=='*':  # new slide
				self.setLevel(0)
				line = line[1:].strip()
				if self.inslide:
					outfile.write('\\end{slide}\n')
				self.outfile.write('\n\\begin{slide}{%s}\n'%line)
				self.inslide = True
				pass
			
			elif c=='-':  # new point
				line = line[1:].strip()
				if (len(self.indent)==0 or whitespace>self.indent[-1]):
					self.addLevel(whitespace)
					self.outfile.write('  '*len(self.indent)+'\\begin{itemize}\n')
					self.outfile.write('  '*len(self.indent)+'\\item '+line+'\n')
				elif whitespace in self.indent:
					self.setLevel(self.indent.index(whitespace)+1)
					self.outfile.write('  '*len(self.indent)+'\\item '+line+'\n')

			elif c=='@':  # preformatted area
				if (self.preformat):
					self.preformat = False
				else:
					self.preformat = True
				self.setLevel(0)
				if self.inslide:
					outfile.write('\\end{slide}\n')
					self.inslide = False
					
			else:  # new line
				if (self.newParagraph):
					self.outfile.write('\n')
				self.outfile.write('  '*len(self.indent))
				if (len(self.indent)>0):
					self.outfile.write('      ')
				self.outfile.write(line+'\n')

			self.newParagraph = False
				
		else:  # blank line
			self.newParagraph = True

if __name__=='__main__':
	input_filename = None
	output_filename = None
	if len(sys.argv)<2:
		print 'Usage: txt2prosper <input.txt> [output.tex]'
		sys.exit(0)
	if len(sys.argv)>=2:
		input_filename = sys.argv[1]
	if len(sys.argv)>=3:
		output_filename = sys.argv[2]

	if input_filename is not None:
		infile = open(input_filename, 'r')
	if output_filename is not None:
		outfile = open(output_filename, 'w')
	else:
		outfile = sys.stdout

	txt2prosper().convert(infile, outfile)
