from bs4 import BeautifulSoup
import requests
import shutil
import csv

import MAUtils
import MADiscogScrapping

hdrs = {'User-Agent': 'Mozilla / 5.0 (X11 Linux x86_64) AppleWebKit / 537.36 '
'(KHTML, like Gecko) Chrome / 52.0.2743.116 '
'Safari / 537.36'
}


class LineupMember:
	def __init__(self, row_elems, band_id, is_current, is_live):
		self.url = row_elems[0].find('a').get('href')
		self.name = row_elems[0].find('a').text
		self.artist_id = self.url[self.url.rfind('/') + 1:]
		self.band_id = band_id
		memb_info = MAUtils.clean_str(row_elems[1].text)
		self.roles = memb_info
		self.periods = ''
		if '(' in memb_info:
			self.roles = memb_info[:memb_info.find('(')]
			self.periods = memb_info[memb_info.find('(') + 1:memb_info.find(')')]
		self.is_current = is_current
		self.is_live = is_live

	def __str__(self):
		return '{:>25} {:>10} {:>20}\n'.format(self.name,self.artist_id,self.roles)

	def writeInCsv(self, writer):
		#  Format: artist_id, band_id, artist_name, artist_roles, artists_periods, is_current (bool) is_live(bool)
		data = [self.artist_id,self.band_id,self.name,self.roles,self.periods,self.is_current,self.is_live]
		writer.writerow(data)

class Band:
	def __init__(self, soup, logofolder = '', find_reviews = True, find_similar = True):
		self.getheaderinfos(soup)
		self.logo_file_path = ''
		if logofolder != '':
		  self.savelogo(soup, logofolder)
		self.lineup = []
		self.getlineupmembers(soup)
		self.albums = MADiscogScrapping.getdiscography(soup, self.ma_id, find_reviews)
		self.similar_bands = [] # list of id/url/score
		if find_similar:
			self.getSimilarBands()

	def getheaderinfos(self, soup):
		band_info = soup.find('div', {'id': "band_info"})
		band_link_elem = band_info.find('h1', {'class': "band_name"}).find('a')
		self.name = MAUtils.clean_str(band_link_elem.text)
		self.ma_id = band_link_elem.get('href')[band_link_elem.get('href').rfind('/') + 1:]
		band_stats_list = band_info.find('div', {'id': "band_stats"}).findAll('dd')
		self.country = band_stats_list[0].text
		self.location = band_stats_list[1].text
		self.status = band_stats_list[2].text
		self.formedin = band_stats_list[3].text
		self.genres = band_stats_list[4].text
		self.themes = band_stats_list[5].text
		self.current_label = band_stats_list[6].text
		self.years_active = MAUtils.clean_str(band_stats_list[7].text)

	def savelogo(self, soup, logofolder):
		logo_url = soup.findAll('img')[1].get('src')
		type_logo_img = logo_url[logo_url.rfind('.') + 1:logo_url.rfind('?')]
		logo_resp = requests.get(logo_url, headers=hdrs, stream=True)
		logo_resp.raw.decode_content = True
		self.logo_file_path = logofolder + self.name + "_logo." + type_logo_img
		logo_file = open(self.logo_file_path, 'wb')
		shutil.copyfileobj(logo_resp.raw, logo_file)
		logo_file.close()
		del logo_resp

	def getdiscography(self, soup, find_reviews):
		discog_url = soup.find('div', {'id': 'band_disco'}).find('li').find('a').get('href')
		discog_resp = requests.get(discog_url, headers=hdrs)
		discog_soup = BeautifulSoup(discog_resp, 'html.parser')
		discog_table = discog_soup.find('table', {'class': 'display discog'}).find('tbody')
		discog_table_rows = discog_table.findAll('tr')
		for discog_table_row in discog_table_rows:
			self.albums.append(Album(discog_table_row, self.ma_id, find_reviews))

	def getlineupmembers(self, soup):
		lineup_table = soup.find("table", {"class": "display lineupTable"})
		if lineup_table is None:
			return
		lineup_table_rows = lineup_table.findAll("tr")
		temp_is_current = False
		temp_is_live = False
		lineup_headers_map = {'Current': (True, False),
							'Lastknown': (True, False),
							'Past': (False, False),
		                    'Current(Live)': (True, True),
							'Lastknown(Live)': (True, True),
							'Past(Live)': (False, True)}
		for lineup_table_row in lineup_table_rows:
			if lineup_table_row.get('class')[0] == 'lineupHeaders':
				headers_cat = MAUtils.clean_str(lineup_table_row.find('td').text).replace(' ','')
				temp_is_current, temp_is_live = lineup_headers_map[headers_cat]
			elif lineup_table_row.get('class')[0] == 'lineupRow':
				row_elems = lineup_table_row.findAll('td')
				self.lineup.append(LineupMember(row_elems, self.ma_id, temp_is_current, temp_is_live))

	def getSimilarBands(self):
		self.similar_bands = []
		base_url = 'https://www.metal-archives.com/band/ajax-recommendations/id/'
		url_sb = base_url + str(self.ma_id)
		resp = requests.get(url_sb, headers=hdrs, params = {'showMoreSimilar': '1'})
		html = resp.content
		soup = BeautifulSoup(html, 'html.parser')
		table_bands = soup.find('table', {'id': 'artist_list'}).find('tbody')
		rows_table_bands = table_bands.findAll('tr')

		for row_table_bands in rows_table_bands:
			col_sim_band =  row_table_bands.findAll('td')
			if col_sim_band[0].get('id')=='no_artists' or col_sim_band[0].get('id')=='show_more':
				break
			else:
				url_sim_band = col_sim_band[0].find('a').get('href')
				id_sim_band = url_sim_band[url_sim_band.rfind('/') + 1:]
				score = int(col_sim_band[3].text)
				self.similar_bands.append((id_sim_band, url_sim_band, score))

	def __str__(self):
		res = '{} (id: {})\n'.format(self.name,self.ma_id)
		res += "  Country: {:>}\n".format(self.country)
		res += "  Location: {}\n".format(self.location)
		res += "  Status: {}\n".format(self.status)
		res += "  Formed in: {}\n".format(self.formedin)
		res += "  Genres: {}\n".format(self.genres)
		res += "  Themes: {}\n".format(self.themes)
		res += "  Label: {}\n".format(self.current_label)
		res += "  Years active: {}\n".format(str(self.years_active))
		res += "  Path logo: {}\n".format(self.logo_file_path)
		res += '  Lineup :\n'
		res += '    Current:\n'
		for member in self.lineup:
			if member.is_current:
				res += "      " + str(member)
		res += '    Past:\n'
		for member in self.lineup:
			if not member.is_current:
				res += "      " + str(member)
		res += '\n'
		res += ' Albums (name,type,year,nb_reviews,avg_score):\n'
		for album in self.albums:
			res += '    '+str(album)
			for review in album.reviews:
				res += '      '+str(review)
		res += '\n'
		res += 'Similar Bands (id,name,score):\n'
		for sim_band in self.similar_bands:
			band_name = sim_band[1][:sim_band[1].rfind('/')]
			band_name = band_name[band_name.rfind('/') + 1:]
			res += '{:>10} {:>20} {:>3}\n'.format(sim_band[0],band_name,sim_band[2])
		return res

	def writeInCsv(self, writer):
		#  Format: id, name,status,formedin,years,country,location,genres,themes,label,path_logo
		data = [self.ma_id,self.name,self.status,self.formedin,
				self.years_active,self.country,self.location,
				self.genres,self.themes,self.current_label,self.logo_file_path]
		writer.writerow(data)

	def writeSimilarBandsinCsv(self, writer):
		# Format: id_band, id_similar_band, discog_resp
		for sim_bands in self.similar_bands:
			data = [self.ma_id,sim_bands[0],sim_bands[2]]
			writer.writerow(data)
################################################################################
################################################################################


def getBand(url, logo_path = '', find_reviews = True, find_similar = True):
	resp = requests.get(url, headers=hdrs)
	if resp.status_code != 200:
		return None
	html = resp.content
	soup = BeautifulSoup(html, 'html.parser')
	if soup.find('h1', {'class': 'page_title'}) is not None:
	 	print('Error: Multiple bands with url '+url)
	else:
  		band = Band(soup, logofolder = logo_path, find_reviews = find_reviews, find_similar = find_similar)
  		return band


def exportLineup(bands, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for band in bands:
			for member in band.lineup:
				member.writeInCsv(csv_writer)

def exportReviews(bands, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for band in bands:
			for album in band.albums:
				for review in album.reviews:
					review.writeInCsv(csv_writer)

def exportBands(bands, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for band in bands:
			band.writeInCsv(csv_writer)

def exportSimilarBands(bands, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for band in bands:
			band.writeSimilarBandsinCsv(csv_writer)
################################################################################
################################################################################

# Use blank logo_path to discard logos
# logo_path = '/PATH/TO/YOUR/EPIC/METAL/BANDS/LOGO/FOLDER/'
#
# url = 'https://www.metal-archives.com/bands/Weedeater'
# band = getBand(url,'', find_reviews = True, find_similar = True)
# if band is None:
# 	print('Url {} not valid.'.format(url))
# else:
# 	print(band)

# EXPORT CSV EXAMPLE
#
# csv_lineup_path = './lineups.csv'
# exportLineup([band], csv_lineup_path)
#
# csv_reviews_path = './album_reviews.csv'
# exportReviews([band], csv_reviews_path)
#
# csv_bands_path   = './bands.csv'
# exportBands([band], csv_bands_path)

# csv_similar_bands_path = './similar_bands.csv'
# exportSimilarBands([band], csv_similar_bands_path)
