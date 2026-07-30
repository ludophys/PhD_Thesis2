DROP TABLE IF EXISTS `SipmBaseline`;
CREATE TABLE `SipmBaseline` (
  `MinRun` int(11) NOT NULL,
  `MaxRun` int(11) DEFAULT NULL,
  `SensorID` int(11) NOT NULL,
  `Energy` float NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
