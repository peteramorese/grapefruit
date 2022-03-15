#include<iostream>
#include<unordered_map>
#include<chrono>
#include<ctime>
#include<fstream>
#include<vector>
#include "benchmark.h"

Benchmark::Benchmark(const std::string& filename_) : filename(filename_) {
	time_start_init = std::chrono::system_clock::now();
};

void Benchmark::addAttribute(const std::string& attr) {
	attributes.push_back(attr);
}
void Benchmark::pushStartPoint(const std::string& name) {
	tp_t time_pt;
	time_pt = std::chrono::system_clock::now();
	time_points_start[name] = time_pt;
}

double Benchmark::measureMilli(const std::string& name) {
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_measure - time_start).count();
	attributes.push_back("-" + name + " (ms): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMilli() {
	tp_t time_measure;
	time_measure = std::chrono::system_clock::now();
	double dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_measure - time_start_init).count();
	attributes.push_back("-init (ms): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMicro(const std::string& name) {
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::microseconds>(time_measure - time_start).count();
	attributes.push_back("-" + name + " (us): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMicro() {
	tp_t time_measure;
	time_measure = std::chrono::system_clock::now();
	double dt = std::chrono::duration_cast<std::chrono::microseconds>(time_measure - time_start_init).count();
	attributes.push_back("-init (us): " + std::to_string(dt));
	return dt;
}

void Benchmark::pushAttributesToFile() {
	std::ofstream F;
	F.open(filename, std::ios::app);
	for (auto attr : attributes) {

		F << attr + "\n";
	}
	F.close();
}

void Benchmark::finishSessionInFile() {
	std::ofstream F;
	F.open(filename, std::ios::app);
	F << ">--\n";
	F.close();
}

void Benchmark::wipeAttributesFromFile() {} //TODO