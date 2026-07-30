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


INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (0, 234, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (235, 619, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (620, 700, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (701, 1072, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (1073, 2452, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2453, 2963, 6, "PMT", "Anode", 15.573, 32.871);

INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 0, "PMT", "Anode", 0.0, 0.0);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 1, "PMT", "Anode", 36.253, 2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 2, "PMT", "Anode", 20.68, -29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 3, "PMT", "Anode", -15.573, -32.871);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 4, "PMT", "Anode", -36.253, -2.949);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 5, "PMT", "Anode", -20.68, 29.922);
INSERT INTO ChannelPosition (MinRun, MaxRun, SensorID, Label, Type, X, Y) VALUES (2964, 9999, 6, "PMT", "Anode", 15.573, 32.871);
