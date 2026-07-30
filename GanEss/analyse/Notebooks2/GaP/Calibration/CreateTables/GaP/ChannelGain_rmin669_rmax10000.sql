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


INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 0, 12.59, 0.1, 15.7, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 1, 7.74, 0.1, 10.8, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 2, 14.84, 0.1, 12.98, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 3, 8.28, 0.1, 10.2, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 4, 13.88, 0.1, 18.82, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 5, 17.3, 0.1, 21.35, 0.1);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (669, 10000, 6, 12.69, 0.1, 17.03, 0.1);
