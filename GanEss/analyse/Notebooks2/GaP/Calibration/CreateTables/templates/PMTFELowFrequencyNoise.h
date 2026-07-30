DROP TABLE IF EXISTS `PMTFELowFrequencyNoise`;
CREATE TABLE `PMTFELowFrequencyNoise` (
	  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number for which valid',
	  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number for which valid',
	  `Frequency` float NOT NULL COMMENT 'Frequency bin centre (Hz)',
	  `FE0Magnitude` float NOT NULL COMMENT 'Frontend Box 0 magnitude (ADC)',
	  `FE1Magnitude` float NOT NULL COMMENT 'Frontend Box 1 magnitude (ADC)',
	  `FE2Magnitude` float NOT NULL COMMENT 'Frontend Box 2 magnitude (ADC)'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
