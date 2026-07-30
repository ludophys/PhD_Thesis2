DROP TABLE IF EXISTS `ChannelMapping`;
CREATE TABLE `ChannelMapping` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
  `ElecID` int(11) NOT NULL COMMENT 'Electronics identifier (eg, DAQ channel number)',
  `SensorID` int(11) NOT NULL COMMENT 'Hardware identifier of a specific sensor'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (0, 234, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (235, 619, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (620, 700, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (701, 1072, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (1073, 2452, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2453, 2963, 6, 6);

INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (2964, 9999, 6, 6);
