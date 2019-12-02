from bs4 import BeautifulSoup
import requests
import csv

import MAUtils

hdrs = {'User-Agent': 'Mozilla / 5.0 (X11 Linux x86_64) AppleWebKit / 537.36 '
'(KHTML, like Gecko) Chrome / 52.0.2743.116 '
'Safari / 537.36'
}

class BandMembership:
    def __init__(self, row_elem, artist_id):
		type_classes = {'artist_tab_active' : 'Active',
                        'artist_tab_past': 'Past',
                        'artist_tab_guest': 'Guest',
		                'artist_tab_misc': "Misc"}
		self.type = type_classes[row_elem.parent.parent.get('id')]
		self.artist_id = artist_id
		bandname_elem = row_elem.find('h3', {'class': 'member_in_band_name'})
		self.bandname = bandname_elem.text
		self.url_band = ''
		self.id_band  = ''
		bandlink_elem = bandname_elem.find('a')
		if bandlink_elem is not None:
			self.url_band = MAUtils.cleanurl(bandlink_elem.get('href'))
			self.id_band  = self.url_band[self.url_band.rfind('/') + 1:]
		bandrole_elem = row_elem.find('p', {'class': 'member_in_band_role'})
		self.role = ''
		if bandrole_elem is not None:
			self.role = MAUtils.clean_str(bandrole_elem.text)

    def __str__(self):
        return self.bandname + ' (id: ' + self.id_band + ') '+self.role+' ('+self.type+')\n'

    def writeInCsv(self, writer):
		#  Format: artist_id, band_id, bandname, artist_role, type_membership
		data = [self.artist_id,self.id_band,self.bandname,self.role,self.type]
		writer.writerow(data)


class Artist:
	def __init__(self, soup, url):
		self.artist_id = url[url.rfind('/') + 1:]
		memb_info = soup.find('div', {'id': 'member_info'})
		self.name = memb_info.find('h1', {'class': 'band_member_name'}).text
		memb_info_sub = memb_info.findAll('dd')
		self.age = MAUtils.clean_str(memb_info_sub[1].text)
		self.origin = MAUtils.clean_str(memb_info_sub[2].text)
		self.gender = MAUtils.clean_str(memb_info_sub[3].text)

		self.memberships = []
		memberships_rows =  soup.findAll('div', {'class': 'member_in_band'})

		for memberships_row in memberships_rows:
			self.memberships.append(BandMembership(memberships_row, self.artist_id))

	def __str__(self):
		res = self.name + ' (id: ' + self.artist_id + ') :' + self.age + ' ' + self.origin+' '+self.gender+'\n'
		for membership in self.memberships:
			res += '  '+str(membership)
		return res

	def writeInCsv(self, writer):
		#  Format: id,name,age,origin,gender
		data = [self.artist_id,self.name,self.age,self.origin,self.gender]
		writer.writerow(data)
################################################################################
################################################################################


def getArtist(url):
    resp = requests.get(url, headers=hdrs)
    html = resp.content
    soup = BeautifulSoup(html, 'html.parser')
    if soup.find('h1', {'class': 'page_title'}) is not None:
        print('Error: Multiple artists with same name')
    else:
  	    artist = Artist(soup, url)
  	    return artist


def exportMemberships(artists, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for artist in artists:
			for membership in artist.memberships:
				membership.writeInCsv(csv_writer)


def exportArtists(artists, path_csv):
	with open(path_csv, 'w') as csv_file:
		csv_writer = csv.writer(csv_file, delimiter=',', quoting = csv.QUOTE_NONNUMERIC)
		for artist in artists:
			artist.writeInCsv(csv_writer)
################################################################################
################################################################################

# url = 'https://www.metal-archives.com/artists/Tundra/2520'
# artist = getArtist(url)
# print(artist)

# EXPORT CSV EXAMPLE
#
# csv_artists_path = './artists.csv'
# exportArtists([artist], csv_artists_path)
#
# csv_memberships_path = '//memberships.csv'
# exportMemberships([artist], csv_memberships_path)
