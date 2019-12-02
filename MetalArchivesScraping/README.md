# Metal (Bands) Mining from [metal-archives](https://www.metal-archives.com/)

The following tools are currently under development!

## Basic Scraping Tools

- [MABandScraping.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScraping/MABandScraping.py) allows you to find extract band information using the band metal-archives url. Right now, it is possible to extract: band general info, band logo, band albums (with m-a album reviews), complete lineup and m-a similar bands  (see usage examples in the python file).

- [MAArtistScraping.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScraping/MAArtistScraping.py) allows you to find extract artist information using the artist metal-archives url. Right now, it is possible to extract: artist general info, band memberships of the artist (see usage examples in the python file)

## Similar Band Network

[MASimilarBandsCrawling.py](https://github.com/fqueyroi/tulip_plugins/tree/master/MetalArchivesScraping/MASimilarBandsCrawling.py) is a Tulip-python Import plugin that generate a network of similar band starting with a given band url. The result is a directed graph: if Metal-archives users find that 'High On Fire' is similar to 'Motorhead', it doesn't mean they find that 'Motorhead' is similar to 'High On Fire'.

Here's an example of output with parameters:
- url : https://www.metal-archives.com/bands/Weedeater/
- min score: 20
- max depth: 1
- logo folder path: a valid folder path

<img src="https://i.imgur.com/vSFSf4Z.png" width="500" height="350">
