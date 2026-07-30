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


INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 0, 29.83, 0.39, 14.7, 0.42);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 1, 15.27, 0.46, 12.81, 0.33);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 2, 25.05, 0.53, 12.28, 0.57);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 3, 22.21, 0.23, -10.68, 0.26);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 4, 25.46, 0.9, 15.43, 0.96);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 5, 34.77, 0.65, 18.18, 0.7);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (0, 234, 6, 31.02, 0.56, 19.0, 0.6);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 0, 22.76, 0.34, 11.89, 0.38);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 1, 12.17, 0.31, 9.35, 0.24);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 2, 23.66, 0.35, 11.87, 0.39);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 3, 17.01, 0.25, 8.62, 0.27);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 4, 22.52, 0.41, 12.58, 0.45);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 5, 27.1, 0.44, 14.08, 0.49);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (235, 619, 6, 20.9, 0.31, 12.18, 0.34);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 0, 16.44, 0.55, -11.15, 0.54);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 1, 9.24, 0.25, -8.05, 0.18);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 2, 18.06, 0.39, -12.49, 0.38);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 3, 11.51, 0.48, 7.07, 0.46);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 4, 14.09, 0.67, -13.37, 0.51);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 5, 19.69, 0.55, -14.78, 0.52);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (620, 700, 6, 17.62, 0.28, 9.95, 0.3);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 0, 12.27, 0.25, 6.68, 0.26);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 1, 7.87, 0.29, 6.44, 0.2);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 2, 14.93, 0.22, 6.78, 0.25);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 3, 9.55, 0.37, -5.8, 0.3);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 4, 13.11, 0.27, 7.51, 0.28);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 5, 17.54, 0.24, 8.1, 0.26);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (701, 1072, 6, 14.72, 0.4, 10.87, 0.36);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 0, 11.12, 0.61, 8.18, 0.45);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 1, 7.3, 0.39, -6.39, 0.22);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 2, 14.71, 0.27, 6.94, 0.3);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 3, 8.52, 0.32, 6.03, 0.26);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 4, 13.11, 0.28, 7.69, 0.29);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 5, 16.09, 0.26, 7.2, 0.28);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (1073, 2452, 6, 12.59, 0.31, 7.92, 0.3);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 0, 11.04, 0.2, 3.87, 0.22);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 1, 9.54, 0.27, 4.61, 0.28);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 2, 6.05, 0.79, 6.15, 0.34);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 3, 14.51, 0.43, 5.14, 0.46);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 4, 14.31, 0.24, 6.7, 0.27);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 5, 11.41, 0.47, 5.56, 0.49);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2453, 2963, 6, 17.77, 0.56, 6.03, 0.61);

INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 0, 8.67, 0.22, 3.51, 0.58);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 1, 7.6, 0.23, 4.14, 0.22);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 2, 5.54, 0.28, -3.15, 0.59);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 3, 10.47, 0.24, 4.43, 0.26);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 4, 10.34, 0.26, 5.54, 0.29);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 5, 7.64, 0.45, 4.52, 0.63);
INSERT INTO ChannelGain(MinRun, MaxRun, SensorID, Centroid, ErrorCentroid, Sigma, ErrorSigma) VALUES (2964, 9999, 6, 13.73, 0.29, 6.58, 0.42);
