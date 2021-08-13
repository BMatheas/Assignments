import argparse, sys, re
import xml.etree.ElementTree as ET


#Funkcia sluzi na kontrolu poctu operandov jednej instrukcie
def arg_cnt(arg, count):
	if len(arg) != count:
		print("Zly pocet operandov u instrukcie")
		sys.exit(32)

#Funckia sluzi na kontrolu a spravnost navestia
def check_label(arg):
	if(arg.attrib["type"] != "label"):
		print("Neplatny typ operandu")
		sys.exit(32)
	x = re.search(r"^([a-z]|[A-Z]|(_|-|\$|&|%|\*|!|\?))([a-z]|[A-Z]|[0-9]|(_|-|\$|&|%|\*|!|\?))*$", arg.text)
	if(x == None):
		print("Chybny label")
		sys.exit(32)

#Funckia sluzi na kontrolu a spravnost symbolu
def check_symb(arg):
	if(arg.attrib["type"] == "var"):
		check_var(arg)
	elif(arg.attrib["type"] == "int"):
		if(arg.text != None):
			x = re.search(r"^(\+|-)?[0-9]+$", arg.text)
			if(x == None):
				print("Chybna hodnota int")
				sys.exit(32)
			arg.text = int(arg.text)
		else:
			arg.text = 0
	elif(arg.attrib["type"] == "string"):
		if(arg.text != None):
			x = re.search(r"^([^\\\s#]|\\\d{3})*$", arg.text)
			if(x == None):
				print("Chybna hodnota string")
				sys.exit(32)
			arg.text = spracuj_esc(arg.text)
		else:
			arg.text = ""
	elif(arg.attrib["type"] == "bool"):
		x = re.search(r"^(false|true)$",arg.text)
		if(x == None):
			print("Chybna hodnota bool")
			sys.exit(32)
		if(arg.text == "false"):
			arg.text = False
		else:
			arg.text = True
	elif(arg.attrib["type"] == "nil"):
		x = re.search(r"^nil$",arg.text)
		if(x == None):
			print("Chybna hodnota nil")
			sys.exit(32)
		arg.text=[]
	else:
		print("Neplatny typ operandu")
		sys.exit(32)	

#Funckia sluzi na prevod escape sekvencie v retazci
def spracuj_esc(arg):
	matches = re.finditer(r"\\\d{1,3}", arg, re.MULTILINE)
	esc = []
	for matchNum, match in enumerate(matches, start=1):
		esc.append(match.group())
	for escape_seq in esc:
		arg = arg.replace(escape_seq, chr(int(escape_seq.lstrip('\\'))))
	return arg

#Funckia sluzi na kontrolu a spravnost symbolu premennej
def check_var(arg):
	if(arg.attrib["type"] != "var"):
		print("Neplatny typ operandu")
		sys.exit(32)
	x = re.search(r"^(GF|LF|TF)@([a-z]|[A-Z]|(_|-|\$|&|%|\*|!|\?))([a-z]|[A-Z]|[0-9]|(_|-|\$|&|%|\*|!|\?))*$", arg.text)
	if(x == None):
		print("Chybny var")
		sys.exit(32)

#Funckia sluzi na kontrolu a spravnost symbolu
def check_type(arg):
	if(arg.attrib["type"] != "type"):
		print("Neplatny typ operandu")
		sys.exit(32)
	x = re.search(r"^(string|int|bool)$", arg.text)
	if(x == None):
		print("Chybny type")
		sys.exit(32)

#Samotna funkcia na kontrolu lexikalnej a syntaktickej spravnosti XML reprezentacie programu
def check_syntax(root):
	for instr in root:
		if instr.attrib["opcode"] in instr_z:
			arg_cnt(instr, 0)
		elif instr.attrib["opcode"] in instr_l:
			arg_cnt(instr, 1)
			check_label(instr[0])
		elif instr.attrib["opcode"] in instr_s:
			arg_cnt(instr, 1)
			check_symb(instr[0])
		elif instr.attrib["opcode"] in instr_v:
			arg_cnt(instr, 1)
			check_var(instr[0])
		elif instr.attrib["opcode"] in instr_vs:
			arg_cnt(instr, 2)
			check_var(instr[0])
			check_symb(instr[1])
		elif instr.attrib["opcode"] in instr_vt:
			arg_cnt(instr, 2)
			check_var(instr[0])
			check_type(instr[1])
		elif instr.attrib["opcode"] in instr_vss:
			arg_cnt(instr, 3)
			check_var(instr[0])
			check_symb(instr[1])
			check_symb(instr[2])
		elif instr.attrib["opcode"] in instr_lss:
			arg_cnt(instr, 3)
			check_label(instr[0])
			check_symb(instr[1])
			check_symb(instr[2])

#Spracovanie XML suboru
def parse_XML(arg):
	try:
		tree = ET.parse(arg)
		xmlroot = tree.getroot()
		xmlroot[:] = sorted(xmlroot, key=lambda child: (child.tag,int(child.get('order'))))
		for c in xmlroot:
			c[:] = sorted(c, key=lambda child: (child.tag,child.tag))
	except:
		print("Zla struktura XML suboru")
		sys.exit(31)
	else:
		return xmlroot

#Kontrola zakladnych parametrov XML suboru
def check_xml_basics(arg, operation):
	order = 1
	if (arg.tag != "program") or ("language" not in arg.attrib) or (arg.attrib["language"] != "IPPcode19"):
		print("Chybna hlavicka")
		sys.exit(32)
	for instr in arg:
		if (instr.tag != "instruction") or (instr.attrib["opcode"] not in operation) or (instr.attrib["order"] != str(order)):
			print("Neznama instrukce alebo chyba atritub alebo spatny order")
			sys.exit(32)
		order += 1

#Spracovanie argumentov
def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument('--source', action="store", default=False, help= "Ocakava vstupny subor s XML reprezentaciou")
	parser.add_argument('--input', action="store", default=False, help="Subor so vstupmi pre interpretovany kod")
	parser.add_argument('--stats', action="store", default=False, help="subor do ktoreho sa budu zbierat statistiky")
	parser.add_argument('--insts', action="store_true", default=False, help="Pocet instrukcii")
	parser.add_argument('--vars', action="store_true", default=False, help="pocet premennych")
	args = parser.parse_args()
	if (args.insts == True or args.vars == True) and (args.stats == False):
		print("Chyba argument --stats")
		sys.exit(10)
	if args.source == False and args.input == False:
		print("Nebol zadany ani source ani input")
		sys.exit(10)		
	return args

#Kontrola redefinicie navesti
def check_redif_labels(arg):
	labels = {}
	for child in arg:
		if(child.attrib["opcode"] == "LABEL"):
			if(child[0].text in labels):
				print("Redefinicia navestia")
				sys.exit(52)
			labels[child[0].text] = int(child.attrib['order'])
	return labels

#Funkcia sluzi na ziskanie hodnoty z premennej
def get_value(frame, var_name):
	if(frame == "GF"):
		if(global_frame[var_name] == None):
			print("Pristup k premennej bez hodnoty")
			sys.exit(56)
		else:
			return global_frame[var_name]
	elif(frame == "LF"):
		if(local_frame[var_name] == None):
			print("Pristup k premennej bez hodnoty")
			sys.exit(56)
		else:
			return local_frame[var_name]
	elif(frame == "TF"):
		if(temporary_frame[var_name] == None):
			print("Pristup k premennej bez hodnoty")
			sys.exit(56)
		else:
			return temporary_frame[var_name]
	else:
		print("Zly ramec")
		sys.exit(55)

#Funcka sluzi pre zapis hodnoty do premennej
def add_val_to_var(frame, var_name, value):
	global var_total
	var_total += 1
	if(frame == "GF"):
		global_frame[var_name] = value
	elif(frame == "LF"):
		local_frame[var_name] = value
	elif(frame == "TF"):
		temporary_frame[var_name] = value
	else:
		print("Zly ramec")
		sys.exit(55)

#Funkcia kontroluje existenicu premennej
def check_var_exist(frame, var_name):
	if(frame == "GF"):
		if(var_name not in global_frame):
			print("Pristup k neexistujucej premennej")
			sys.exit(54)
	elif(frame == "LF"):
		if(local_frame == None):
			print("Ramec neexistuje")
			sys.exit(55)
		elif(var_name not in local_frame):
			print("Pristup k neexistujucej premennej")
			sys.exit(54)
	elif(frame == "TF"):
		if(temporary_frame == None):
			print("Ramec neexistuje")
			sys.exit(55)
		elif(var_name not in temporary_frame):
			print("Pristup k neexistujucej premennej")
			sys.exit(54)
	else:
		print("Zly ramec")
		sys.exit(55)

#Funkcia na samotnu interpretaciu programu, pozostava z for cyklu a podmienenych prikazov if, elif, else. Kde if/elif odpoveda jednej instrukcii
def start_interpret(root):
	global labels, global_frame, temporary_frame, local_frame, stack, frame_stack, call_stack, order, instr_total, var_total, xml_file
	for instr in root:
		instr_total += 1
		order += 1
		
		if(instr.attrib["opcode"] == "MOVE"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0], op1[1])
				val = get_value(op1[0], op1[1])
				add_val_to_var(op0[0], op0[1], val)
			else:
				add_val_to_var(op0[0], op0[1],instr[1].text)
			
		elif(instr.attrib["opcode"] == "CREATEFRAME"):
			temporary_frame = {}

		elif(instr.attrib["opcode"] == "PUSHFRAME"):
			if(temporary_frame == None):
				print("Neexistujuci ramec")
				sys.exit(55)
			frame_stack.append(temporary_frame)
			local_frame = frame_stack[len(frame_stack)-1]
			temporary_frame = None

		elif(instr.attrib["opcode"] == "POPFRAME"):
			if(local_frame == None):
				print("Neexistujuci ramec")
				sys.exit(55)
			temporary_frame = frame_stack.pop()
			if(len(frame_stack) != 0):
				local_frame = frame_stack[len(frame_stack)-1]
			else:
				local_frame = None

		elif(instr.attrib["opcode"] == "DEFVAR"):
			variable = instr[0].text
			variable = variable.split('@',1)
			if(variable[0] == "GF"):
				global_frame[variable[1]] = None
			elif (variable[0] == "TF"):
				if(temporary_frame == None):
					print("Neexistujuci ramec")
					sys.exit(55)
				else:
					temporary_frame[variable[1]] = None
			elif(variable[0] == "LF"):
				if(local_frame == None):
					print("Neexistujuci ramec")
					sys.exit(55)
				else:
					local_frame[variable[1]] = None
			else:
				print("Invalidny nazov ramca")
				sys.exit(55)

		elif(instr.attrib["opcode"] == "CALL"):
			if(instr[0].text not in labels):
				print("Neexistujuce navestie")
				exit(52)
			call_stack.append(int(instr.attrib["order"]))
			start_interpret(xml_file[(labels[instr[0].text])-1:])
			break

		elif(instr.attrib["opcode"] == "RETURN"):
			if(call_stack == []):
				print("Zasobnik prazdny")
				sys.exit(56)	
			start_interpret(xml_file[(call_stack.pop()):])
			break

		elif(instr.attrib["opcode"] == "PUSHS"):
			if(instr[0].attrib["type"] == "var"):
				op0 = instr[0].text.split('@')
				check_var_exist(op0[0], op0[1])
				val = get_value(op0[0], op0[1])
				stack.append(val)
			else:
				stack.append(instr[0].text)

		elif(instr.attrib["opcode"] == "POPS"):
			if(stack == []):
				print("Prazdny zasobnik")
				sys.exit(56)
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			val = stack.pop()
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "ADD"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0], op1[1])
				val1 = get_value(op1[0], op1[1])
				if(type(val1) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[1].attrib["type"] == "int"):
					val1 = instr[1].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0], op2[1])
				val2 = get_value(op2[0], op2[1])
				if(type(val2) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[2].attrib["type"] == "int"):
					val2 = instr[2].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)
			add_val_to_var(op0[0], op0[1], val1+val2)

		elif(instr.attrib["opcode"] == "SUB"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0], op1[1])
				val1 = get_value(op1[0], op1[1])
				if(type(val1) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[1].attrib["type"] == "int"):
					val1 = instr[1].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0], op2[1])
				val2 = get_value(op2[0], op2[1])
				if(type(val2) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[2].attrib["type"] == "int"):
					val2 = instr[2].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)
			add_val_to_var(op0[0], op0[1], val1-val2)

		elif(instr.attrib["opcode"] == "MUL"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0], op1[1])
				val1 = get_value(op1[0], op1[1])
				if(type(val1) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[1].attrib["type"] == "int"):
					val1 = instr[1].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0], op2[1])
				val2 = get_value(op2[0], op2[1])
				if(type(val2) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[2].attrib["type"] == "int"):
					val2 = instr[2].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)
			add_val_to_var(op0[0], op0[1], val1*val2)

		elif(instr.attrib["opcode"] == "IDIV"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0], op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0], op1[1])
				val1 = get_value(op1[0], op1[1])
				if(type(val1) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[1].attrib["type"] == "int"):
					val1 = instr[1].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0], op2[1])
				val2 = get_value(op2[0], op2[1])
				if(type(val2) is not int):
					print("Neplatna hodnota operandu")
					sys.exit(53)
			else:
				if(instr[2].attrib["type"] == "int"):
					val2 = instr[2].text
				else:
					print("Neplatny typ operandu")
					sys.exit(53)

			if(val2 == 0):
				print("Delenie nulou")
				sys.exit(57)
			add_val_to_var(op0[0], op0[1], val1//val2)
	
		elif(instr.attrib["opcode"] == "LT"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val1) is list or type(val2) is list):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)
			if(type(val1) != type(val2)):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)
			val = val1 < val2
			add_val_to_var(op0[0], op0[1], val)
	
		elif(instr.attrib["opcode"] == "GT"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val1) is list or type(val2) is list):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)
			if(type(val1) != type(val2) ):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)
			val = val1 > val2
			add_val_to_var(op0[0], op0[1], val)
				
		elif(instr.attrib["opcode"] == "EQ"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val1) is list):
				val = [] == val2
				add_val_to_var(op0[0], op0[1], val)	
			elif(type(val2) is list):
				val = [] == val1
				add_val_to_var(op0[0], op0[1], val)	
			else:
				if(type(val2) != type(val1)):
					print("Instr nema rovnake typy operandov")
					sys.exit(53)
				val = val1 == val2
				add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "AND"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text

			if(type(val1) is not bool or type(val2) is not bool):
				print("Instr nema operandy typu bool")
				sys.exit(53)
			val = val1 and val2
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "OR"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text

			if(type(val1) is not bool or type(val2) is not bool):
				print("Instr nema operandy typu bool")
				sys.exit(53)
			val = val1 or val2
			add_val_to_var(op0[0], op0[1], val)
	
		elif(instr.attrib["opcode"] == "NOT"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(type(val1) is not bool):
				print("Instr nema operandy typu bool")
				sys.exit(53)
			val = not val1
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "INT2CHAR"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text

			if(type(val1) is not int):
				print("Zly typ operandu")
				sys.exit(53)

			if(val1 < 0 or val1 > 1114111):
				print("CHR mimo rozsah")
				sys.exit(58)
			val = chr(val1)
			add_val_to_var(op0[0], op0[1], val)
			
		elif(instr.attrib["opcode"] == "STRI2INT"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text

			if(type(val1) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(type(val2) is not int):
				print("Nespravny typ operandu")
				sys.exit(53)
			if((len(val1) <= val2) or (val2 < 0)):
				print("Mimo index stringu")
				sys.exit(58)
			val = ord(val1[val2])
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "READ"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] != "type"):
				print("Nespravny typ operandu")
				sys.exit(53)
			try:
				read_input = input_file.readline().rstrip('\r\n')
			except:
				if(instr[1].text == "int"):
					val = 0;
				elif(instr[1].text == "string"):
					val = "";
				elif(instr[1].text == "bool"):
					val = False
				else:
					print("Type nema spravnu hodnotu")
					sys.exit(57)
			else:
				if(instr[1].text == "int"):
					try:
						int(read_input)
					except:
						val = 0
					else:
						val = int(read_input)
				elif(instr[1].text == "string"):
						val = read_input
				elif(instr[1].text == "bool"):
					if(read_input.lower() in ['true']):
						val = True
					else:
						val = False
				else:
					print("Type nema spravnu hodnotu")
					sys.exit(57)
			add_val_to_var(op0[0],op0[1],val)

		elif(instr.attrib["opcode"] == "WRITE"):
			if(instr[0].attrib["type"] == "var"):
				op0 = instr[0].text.split('@')
				check_var_exist(op0[0], op0[1])
				val = get_value(op0[0], op0[1])
			else:
				val = instr[0].text
			if(instr[0].attrib["type"] == "nil"):
				print("",end="")	
			elif(type(val) is bool):
				if(val == False):
					print("false", end="")
				else:
					print("true", end="")
			elif(type(val) is list):
				print("",end="")
			else:
				print(val, end="")

		elif(instr.attrib["opcode"] == "CONCAT"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val1) is not str or type(val2) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			val = val1 + val2
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "STRLEN"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(type(val1) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			val = len(val1)
			add_val_to_var(op0[0], op0[1], val)
			
		elif(instr.attrib["opcode"] == "GETCHAR"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text

			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text

			if(type(val1) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(type(val2) is not int):
				print("Nespravny typ operandu")
				sys.exit(53)
			if((len(val1) <= val2) or (val2 < 0)):
				print("Mimo index stringu")
				sys.exit(58)
			val = val1[val2]
			add_val_to_var(op0[0], op0[1], val)
			
		elif(instr.attrib["opcode"] == "SETCHAR"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			val0 = get_value(op0[0],op0[1])
			if(type(val0) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val1) is not int):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(type(val2) is not str):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(len(val2) == 0):
				print("Prazdny retazec")
				sys.exit(58)
			if((len(val2) <= val1) or (val1 < 0)):
				print("Mimo index stringu")
				sys.exit(58)
			if(len(val2) == 1):
				val = val0.replace(val0[val1],val2)
			else:
				val = val0.replace(val0[val1],val2[0])
			add_val_to_var(op0[0], op0[1], val)

		elif(instr.attrib["opcode"] == "TYPE"):
			op0 = instr[0].text.split('@')
			check_var_exist(op0[0],op0[1])
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[1].attrib["type"] == "nil"):
				add_val_to_var(op0[0], op0[1], "nil")
			elif(type(val1) is list):
				add_val_to_var(op0[0], op0[1], "nil")
			elif(type(val1) is int):
				add_val_to_var(op0[0], op0[1], "int")
			elif(type(val1) is str):
				add_val_to_var(op0[0], op0[1], "string")
			elif(type(val1) is bool):
				add_val_to_var(op0[0], op0[1], "bool")
		
		elif(instr.attrib["opcode"] == "LABEL"):
			continue

		elif(instr.attrib["opcode"] == "JUMP"):
			if(instr[0].text not in labels):
				print("Skok na zle navestie")
				sys.exit(52)
			start_interpret(xml_file[(labels[instr[0].text])-1:])
			break;

		elif(instr.attrib["opcode"] == "JUMPIFEQ"):
			if(instr[0].text not in labels):
				print("Skok na zle navestie")
				sys.exit(52)
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val2) != type(val1)):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)

			if(val1 == val2):
				start_interpret(xml_file[(labels[instr[0].text])-1:])
				break

		elif(instr.attrib["opcode"] == "JUMPIFNEQ"):
			if(instr[0].text not in labels):
				print("Skok na zle navestie")
				sys.exit(52)
			if(instr[1].attrib["type"] == "var"):
				op1 = instr[1].text.split('@')
				check_var_exist(op1[0],op1[1])
				val1 = get_value(op1[0],op1[1])
			else:
				val1 = instr[1].text
			if(instr[2].attrib["type"] == "var"):
				op2 = instr[2].text.split('@')
				check_var_exist(op2[0],op2[1])
				val2 = get_value(op2[0],op2[1])
			else:
				val2 = instr[2].text
			if(type(val2) != type(val1)):
				print("Instr nema rovnake typy operandov")
				sys.exit(53)
				
			if(val1 != val2):
				start_interpret(xml_file[(labels[instr[0].text])-1:])
				break

		elif(instr.attrib["opcode"] == "EXIT"):
			if(instr[0].attrib["type"] == "var"):
				op0 = instr[0].text.split('@')
				check_var_exist(op0[0],op0[1])
				val0 = get_value(op0[0],op0[1])
			else:
				val0 = instr[0].text
			if(type(val0) is not int):
				print("Nespravny typ operandu")
				sys.exit(53)
			if(val0 >= 50 or val0 < 0):
				print("Neplatny rozsah u exit")
				sys.exit(57)

			if arg_val.stats != False:
				stat_file = open(arg_val.stats,'w')
				for arg in sys.argv:
					if(arg == "--insts"):
						print(instr_total, file=stat_file)
					if(arg == "--vars"):
						print(var_total, file=stat_file)
			sys.exit(val0)

		elif(instr.attrib["opcode"] == "DPRINT"):
			if(instr[0].attrib["type"] == "var"):
				op0 = instr[0].text.split('@')
				check_var_exist(op0[0],op0[1])
				val0 = get_value(op0[0],op0[1])
			else:
				val0 = instr[0].text
			if(type(val0) is list):
				sys.stderr.write("")
			elif(type(val0) is bool):
				if(val0 == False):
					sys.stderr.write("false")
				else:
					sys.stderr.write("true")
			else:
				sys.stderr.write(val0)

		elif(instr.attrib["opcode"] == "BREAK"):
			sys.stderr.write("Pocet spracovanych instrukcii: {}\n".format(instr_total))
			sys.stderr.write("Obsah GF: {}\n".format(global_frame))
			sys.stderr.write("Obsah LF: {}\n".format(local_frame))
			sys.stderr.write("Obsah TF: {}\n".format(temporary_frame))
		else:
			print("Neplatny nazov instrukcie")
			sys.exit(32)

# start programu
''' pomocne pole pre kontrolu XML '''
instr_arr = ["MOVE","CREATEFRAME","PUSHFRAME","POPFRAME","DEFVAR","CALL","RETURN","PUSHS","POPS","ADD","SUB","MUL","IDIV","LT","GT","EQ","AND","OR","NOT",
             "INT2CHAR","STRI2INT","READ","WRITE","CONCAT","STRLEN","GETCHAR","SETCHAR","TYPE","LABEL","JUMP","JUMPIFEQ","JUMPIFNEQ","EXIT","DPRINT","BREAK"]
instr_z = ["CREATEFRAME", "PUSHFRAME", "POPFRAME", "RETURN", "BREAK"]
instr_l = ["CALL", "LABEL", "JUMP"]
instr_s = ["PUSHS", "WRITE", "DPRINT","EXIT"]
instr_v = ["DEFVAR", "POPS"]
instr_vs = ["MOVE", "INT2CHAR", "STRLEN", "TYPE", "NOT"]
instr_vt = ["READ"]
instr_vss = ["ADD", "SUB", "MUL", "IDIV", "LT", "GT", "EQ", "AND", "OR","STRI2INT", "CONCAT", "GETCHAR", "SETCHAR"]
instr_lss = ["JUMPIFEQ", "JUMPIFNEQ"]

#kontrola argumentov
arg_val = parse_args()
source_file = None
input_file = None
stat_file = None
if arg_val.source != False and arg_val.input == False:
	input_file = sys.stdin
	try:
		source_file = open(arg_val.source,'r')
	except:
		sys.exit(11)
elif arg_val.source != False and arg_val.input == True:
	source_file = sys.stdin 
	try:
		input_file = open(arg_val.input, 'r')
	except:
		sys.exit(11)
else:
	source_file = open(arg_val.source,'r')
	input_file = open(arg_val.input, 'r')
xml_file = parse_XML(source_file)

#kontrola XML suboru
check_xml_basics(xml_file,instr_arr)
check_syntax(xml_file)

#spis pouzivanych premennych
labels = check_redif_labels(xml_file)
instr_total = 0
var_total = 0
order = 1
global_frame = {}
local_frame = None
temporary_frame = None
stack = []
frame_stack = []
call_stack = []

#samotny interpret
start_interpret(xml_file)

#vypis statistik STATI do suboru
if arg_val.stats != False:
	stat_file = open(arg_val.stats,'w')
	for arg in sys.argv:
		if(arg == "--insts"):
			print(instr_total, file=stat_file)
		if(arg == "--vars"):
			print(var_total, file=stat_file)
sys.exit(0)
