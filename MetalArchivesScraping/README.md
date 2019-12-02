# Metal Band Networks from [metal-archives](https://www.metal-archives.com/)

The following tools are currently under development!

## Basic Scrapping Tools

- [MABandScrapping.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScrapping/MABandScrapping.py) allows you to find extract band information using the band metal-archives url. Right now, it is possible to extract: band general info, band logo, band albums (and m-a album reviews), complete lineup and m-a similar bands  (see usage examples in the python file).

- [MAArtistScrapping.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScrapping/MAArtistScrapping.py) allows you to find extract artist information using the artist metal-archives url. Right now, it is possible to extract: artist general info, band memberships of the artist (see usage examples in the python file)

## Similar Band Network

[MASimilarBandsCrawling.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScrapping/MASimilarBandsCrawling.py) is a Tulip-python Import plugin that generate a network of similar band starting with a given band url.

Here's an example of output with parameters:
- url : https://www.metal-archives.com/bands/Weedeater/
- min score: 20
- max depth: 1

![Similar network example](https://i.imgur.com/vSFSf4Z.png)
