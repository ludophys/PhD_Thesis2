DROP TABLE IF EXISTS `ChannelMapping`;
CREATE TABLE `ChannelMapping` (
  `MinRun` int(11) NOT NULL COMMENT 'Minimum run number',
  `MaxRun` int(11) NOT NULL COMMENT 'Maximum run number',
  `ElecID` int(11) NOT NULL COMMENT 'Electronics identifier (eg, DAQ channel number)',
  `SensorID` int(11) NOT NULL COMMENT 'Hardware identifier of a specific sensor'
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 0, 0);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 1, 1);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 2, 2);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 3, 3);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 4, 4);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 5, 5);
INSERT INTO ChannelMapping (MinRun, MaxRun, ElecID, SensorID) VALUES (669, 10000, 6, 6);
