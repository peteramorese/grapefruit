#include<iostream>
#include<unordered_map>
#include<chrono>
#include<ctime>
#include<fstream>
#include<vector>
#include "benchmark.h"

const std::string Benchmark::time_attr_marker = "[T] ";
const std::string Benchmark::tuple_attr_marker = "[Tup] ";

Benchmark::Benchmark(const std::string* filename_) : filename(filename_) {
	if (filename) time_start_init = std::chrono::system_clock::now();
};

void Benchmark::addAttribute(const std::string& attr, const std::string& attr_val) {
	if (!filename) return;
	attributes.push_back(attr + ": " + attr_val);
}

void Benchmark::addAttribute(const std::string& attr, const std::string& attr_val1, const std::string& attr_val2) {
	if (!filename) return;
	attributes.push_back(tuple_attr_marker + attr + ": {" + attr_val1 +", " + attr_val2 + "}");
}

void Benchmark::addCustomTimeAttr(const std::string& attr, double custom_time, const std::string& units) {
	if (!filename) return;
    attributes.push_back(time_attr_marker + attr + " (" + units + "): " + std::to_string(custom_time));
}

void Benchmark::pushStartPoint(const std::string& name) {
	if (!filename) return;
	tp_t time_pt;
	time_pt = std::chrono::system_clock::now();
	time_points_start[name] = time_pt;
}

double Benchmark::measureMilli(const std::string& name) {
	if (!filename) return -1.0f;
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_measure - time_start).count();
	attributes.push_back(time_attr_marker + name + " (ms): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMilli(const std::string& name, const std::string& attr_val2) {
	if (!filename) return -1.0f;
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_measure - time_start).count();
	attributes.push_back(tuple_attr_marker + name + " (ms): {" + std::to_string(dt) + ", " + attr_val2 + "}");
	return dt;
}

double Benchmark::measureMilli() {
	if (!filename) return -1.0f;
	tp_t time_measure;
	time_measure = std::chrono::system_clock::now();
	double dt = std::chrono::duration_cast<std::chrono::milliseconds>(time_measure - time_start_init).count();
	attributes.push_back(time_attr_marker + "init (ms): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMicro(const std::string& name) {
	if (!filename) return -1.0f;
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::microseconds>(time_measure - time_start).count();
	attributes.push_back(time_attr_marker + name + " (us): " + std::to_string(dt));
	return dt;
}

double Benchmark::measureMicro(const std::string& name, const std::string& attr_val2) {
	if (!filename) return -1.0f;
	tp_t time_measure, time_start;
	time_measure = std::chrono::system_clock::now();
	time_start = time_points_start.at(name);
	double dt = std::chrono::duration_cast<std::chrono::microseconds>(time_measure - time_start).count();
	attributes.push_back(tuple_attr_marker + name + " (us): " + std::to_string(dt) + ", " + attr_val2 + "}");
	return dt;
}

double Benchmark::measureMicro() {
	if (!filename) return -1.0f;
	tp_t time_measure;
	time_measure = std::chrono::system_clock::now();
	double dt = std::chrono::duration_cast<std::chrono::microseconds>(time_measure - time_start_init).count();
	attributes.push_back(time_attr_marker + "init (us): " + std::to_string(dt));
	return dt;
}

void Benchmark::pushAttributesToFile() {
	if (!filename) return;
	std::ofstream F;
	F.open(*filename, std::ios::app);
	for (auto attr : attributes) {

		F << attr + "\n";
	}
	F.close();
}

void Benchmark::finishSessionInFile() {
	if (!filename) return;
	std::ofstream F;
	F.open(*filename, std::ios::app);
	F << ">--\n";
	F.close();
}

void Benchmark::wipeAttributesFromFile() {} //TODO