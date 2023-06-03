# TSyncOneWire
# https://github.com/skerr92/TSyncOneWire
# MIT License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "TSyncOneWire.cpp" 
                       INCLUDE_DIRS "."
                       REQUIRES arduino)

project(TSyncOneWire)
