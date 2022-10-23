# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#idf_component_register(SRCS ${app_sources})
idf_component_register(
  SRCS "dy_sv17f.c" "mqtt.c" "wifi_station.c" "lvgl_key.c" "app_main.c""my_dns_server.c" "webserver.c"  "mpu6050.c" "updata.c"
  #"st7735s.c"            
  INCLUDE_DIRS "."
  EMBED_FILES  "index.html")
 #register_component()