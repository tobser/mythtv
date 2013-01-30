delimiter $$
DROP table `mythconverg`.`teatime`$$
DROP table `mythconverg`.`teatime_rundata`$$
CREATE TABLE `teatime`  (
  `id` MEDIUMINT NOT NULL AUTO_INCREMENT,
   PRIMARY KEY(`id`),
  `message_text` text,  
  `exec_date_time` timestamp NULL DEFAULT NULL COMMENT 'time the timer actions will be executed',
  `date_time` timestamp NULL DEFAULT NULL COMMENT 'Either this or \'time_span\' have to contain valid data',
  `time_span` time DEFAULT NULL COMMENT 'Either this or \'date_time\' have to contain valid data',
  `pause_playback` enum('no','yes') DEFAULT NULL,
  `countdown_seconds` int(11) DEFAULT NULL
  COMMENT 'stores timer data for the teatime plugin'
) ENGINE=MyISAM DEFAULT CHARSET=utf8$$

CREATE TABLE `teatime_rundata`  (
	`timer_id` MEDIUMINT NOT NULL,   
	`run_order` TINYINT NOT NULL,
	`type` enum('jump_point','system_key_event') NOT NULL,
	`data` text NOT NULL
	COMMENT 'stores what shall be executed and in which order when the timer is expired'
) ENGINE=MyISAM DEFAULT CHARSET=utf8
$$

