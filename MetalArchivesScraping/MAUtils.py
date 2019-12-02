
def del_eol(string):
	res = string.replace(u'\n',u'')
	return res

def clean_str(string):
	res = string
	res = res.replace(u'\xa0', u' ')
	res = res.replace(u'\xc3\u0153',u'U')
	res = res.replace(u'\u2026',u'...')
	res = res.replace(u'\u2019',u'')
	res = res.replace(u'\u201c',u'"')
	res = res.replace(u'\u201d',u'"')
	res = res.replace(u'\u2013',u'-')
	res = res.replace(u'\u2022',u'-')
	res = res.encode("utf-8")
	res = res.replace('\n','')
	return res.strip()

def cleanurl(url):
	pos_jump_sym = url.find('#')
	return url[:pos_jump_sym]

# test = u'Buzzov\u2022en'
# print(repr(test))
# print(test.find(u'\u2022'))
# print(test.replace(u'\u2022',u'-'))
# print(repr(clean_str(test)))
