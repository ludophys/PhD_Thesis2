DROP TABLE IF EXISTS `SipmNoisePDF`;
CREATE TABLE `SipmNoisePDF` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) DEFAULT NULL COMMENT 'Maximum run number',
  `SensorID` int(11) NOT NULL COMMENT 'Sensor identifier',
  `BinEnergyPes` float NOT NULL,
  `Probability` float NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

