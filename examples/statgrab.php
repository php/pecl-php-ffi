<?php

/* Interface to libstatgrab library via ffi */


class statg extends ffi
{
	function __construct()
	{
		parent::__construct(<<<EOD
struct load_stat_t {
	double min1;
	double min5;
	double min15;
};

struct cpu_percent_t {
	float user;
	float kernel;
	float idle;
	float iowait;
	float swap;
	float nice;
	long time_taken;
};

struct cpu_states_t {
	sint64 user;
	sint64 kernel;
	sint64 idle;
	sint64 iowait;
	sint64 swap;
	sint64 nice;
	sint64 total;
	long systime;
};

struct diskio_stat_t {
	char *disk_name;
	sint64 read_bytes;
	sint64 write_bytes;
	long systime;
};

struct general_stat_t {
	char *os_name;
	char *os_release;
	char *os_version;
	char *platform;
	char *hostname;
	long uptime;
};

struct mem_stat_t {
	sint64 total;
	sint64 free;
	sint64 used;
	sint64 cache;
};

struct swap_stat_t {
	sint64 total;
	sint64 used;
	sint64 free;
};

struct network_stat_t {
	char *interface_name;
	sint64 tx;
	sint64 rx;
	long systime;
};

struct page_stat_t {
	sint64 pages_pagein;
	sint64 pages_pageout;
	long systime;
};

struct process_stat_t {
	int total;
	int running;
	int sleeping;
	int stopped;
	int zombie;
};

struct user_stat_t {
	char *name_list;
	int num_entries;
};

[lib='libstatgrab.so'] 
	struct load_stat_t *get_load_stats();
	struct cpu_percent_t *cpu_percent_usage();
	struct cpu_states_t *get_cpu_totals();
	struct cpu_states_t *get_cpu_diff();
	struct diskio_stat_t *get_diskio_stats(int entries);
	struct diskio_stat_t *get_diskio_stats_diff(int entries);
	struct general_stat_t *get_general_stats();
	struct mem_stat_t *get_memory_stats();
	struct swap_stat_t *get_swap_stats();
	struct network_stat_t *get_network_stats(int entries);
	struct network_stat_t *get_network_stats_diff(int entries);
	struct page_stat_t *get_page_stats();
	struct page_stat_t *get_page_stats_diff();
	struct process_stat_t *get_process_stats();
	struct user_stat_t *get_user_stats();
	void statgrab_init();
	int statgrab_drop_privileges();

EOD
		
		);

		/* some initialization routines */
		$this->statgrab_init();
		if ($this->statgrab_drop_privileges() != 0) {
			throw new Exception('Failed to initialize statgrab');
		}
	}
}

/* Sample usage */

$ffi = new statg();

$ls = $ffi->get_load_stats();
var_dump($ls->min1, $ls->min5, $ls->min15);

$ps = $ffi->get_process_stats();
var_dump($ps->total);

$us = $ffi->get_user_stats();
var_dump($us->name_list);
?>