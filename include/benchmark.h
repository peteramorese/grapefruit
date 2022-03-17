#pragma once
#include<iostream>
#include<unordered_map>
#include<chrono>
#include<ctime>
#include<fstream>
#include<vector>

class Benchmark {
	private:
		using tp_t = std::chrono::time_point<std::chrono::system_clock>;
		tp_t time_start_init;
		std::unordered_map<std::string, tp_t> time_points_start;
		std::vector<std::string> attributes;
		const std::string filename;
	public:
		static const std::string time_attr_marker;
	 	Benchmark(const std::string& filename_);
		void addAttribute(const std::string& attr);
		void addCustomTimeAttr(const std::string& attr, double custom_time, const std::string& units);
		void pushStartPoint(const std::string& name);
		double measureMilli(const std::string& name);
		double measureMilli();
		double measureMicro(const std::string& name);
		double measureMicro();
		void pushAttributesToFile();
		void finishSessionInFile();
		void wipeAttributesFromFile();
};