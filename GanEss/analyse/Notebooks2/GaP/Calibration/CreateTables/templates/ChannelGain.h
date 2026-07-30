DROP TABLE IF EXISTS `ChannelGain`;

CREATE TABLE `ChannelGain` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
  `SensorID` int(11) NOT NULL COMMENT 'Sensor identifier',
  `Centroid` float NOT NULL COMMENT 'Best estimate for the gain. For cathode channels, this is the peak position of the single-PE distribution',
  `ErrorCentroid` float NOT NULL COMMENT 'Uncertainty on the gain. For cathode channels, this is the uncertainty on how well we know the peak position of the single-PE distribution',
  `Sigma` float NOT NULL COMMENT 'Best estimate for the gain spread. For cathode channels, the gain spread is width of the single-PE distribution',
  `ErrorSigma` float NOT NULL COMMENT 'Error on the gain spread. For cathode channels, this is the uncertainty on how well we know the width of the single-PE distribution',
  KEY `ElecID` (`SensorID`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
