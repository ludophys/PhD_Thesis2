DROP TABLE IF EXISTS `ChannelPosition`;
CREATE TABLE `ChannelPosition` (
	  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
	  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
	  `SensorID` int(11) NOT NULL COMMENT 'Hardware identifier of a specific sensor',
	  `Label` varchar(20) NOT NULL COMMENT 'Name of this sensor',
	  `Type` varchar(20) NOT NULL COMMENT 'Can be: "Cathode", "Anode" or "NaI"',
	  `X` float NOT NULL COMMENT 'Sensor x position (mm)',
	  `Y` float NOT NULL COMMENT 'Sensor y position (mm)'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 2, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 3, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 4, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 5, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 669, 6, "PMT", "Anode", 15.573, 32.871);
