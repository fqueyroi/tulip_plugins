from bs4 import BeautifulSoup
import requests
import csv

import MAUtils

hdrs = {'User-Agent': 'Mozilla / 5.0 (X11 Linux x86_64) AppleWebKit / 537.36 '
'(KHTML, like Gecko) Chrome / 52.0.2743.116 '
'Safari / 537.36'
}

class AlbumReview:
	def __init__(self, review_box, band_id, album_id):
		self.band_id = band_id
		self.album_id = album_id

		title_content = review_box.find('h3', {'class': 'reviewTitle'}).text
		self.title    = MAUtils.clean_str(title_content[:title_content.rfind(" - ")])
		self.rating   = int(MAUtils.clean_str(title_content[title_content.rfind(" - ") + 3:title_content.find('%')]))

		profile_content = review_box.find('a', {'class': 'profileMenu'}).parent.text
		self.username   = MAUtils.clean_str(profile_content[:profile_content.find(',')])
		end_line_index = len(profile_content)
		if profile_content.find('Written based on this version:') != -1:
			end_line_index = profile_content.find('Written based on this version:')
		self.date       = MAUtils.clean_str(profile_content[profile_content.find(',') + 1 : end_line_index])

	def __str__(self):
		return '{:>25} {:>3} {:>20} {:50}\n'.format(self.username,self.rating,self.date,self.title)

	def writeInCsv(self, writer):
		#  Format: username, band_id, album_id, date, review_rating, review_title
		data = [self.username,self.band_id,self.album_id,self.date,self.rating,self.title]
		writer.writerow(data)


class Album:
	def __init__(self, row_elems, band_id, find_reviews = True):
		self.band_id = band_id
		col_elems = row_elems.findAll('td')
		self.url = col_elems[0].find('a').get('href')
		self.name = MAUtils.clean_str(col_elems[0].find('a').text)
		self.album_id = self.url[self.url.rfind('/') + 1:]
		self.type = col_elems[1].text
		self.year = col_elems[2].text
		self.nb_reviews = 0
		self.rating = 0
		review_txt = MAUtils.clean_str(col_elems[3].text)
		self.reviews = []

		if review_txt != '':
			self.nb_reviews = int(review_txt[:review_txt.find('(')])
			self.rating = int(review_txt[review_txt.find('(') + 1:review_txt.find('%')])
			# Get Reviews
			reviews_url = col_elems[3].find('a').get('href')
			if find_reviews:
				self.reviews = self.getreviews(reviews_url)

	def getreviews(self, reviews_url):
		reviews_resp = requests.get(reviews_url, headers=hdrs)
		reviews_soup = BeautifulSoup(reviews_resp.content, 'html.parser')
		reviews_boxes = reviews_soup.findAll('div', {'class': 'reviewBox'})
		reviews = []
		for reviews_box in reviews_boxes:
			reviews.append(AlbumReview(reviews_box, self.band_id, self.album_id))
		return reviews

	def __str__(self):
		res = '{:>35} {:>10} {:>4} {:>3} {:>3}\n'.format(self.name,self.type,self.year,self.nb_reviews,self.rating)
		return res

def getdiscography(soup, band_id, find_reviews):
	albums = []
	discog_url = soup.find('div', {'id': 'band_disco'}).find('li').find('a').get('href')
	discog_resp = requests.get(discog_url, headers=hdrs)
	discog_soup = BeautifulSoup(discog_resp.content, 'html.parser')
	discog_table = discog_soup.find('table', {'class': 'display discog'}).find('tbody')
	discog_table_rows = discog_table.findAll('tr')
	for discog_table_row in discog_table_rows:
		albums.append(Album(discog_table_row, band_id, find_reviews))
	return albums
