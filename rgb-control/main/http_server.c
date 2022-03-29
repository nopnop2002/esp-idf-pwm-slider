/* HTTP Server Example

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <mbedtls/base64.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "esp_http_server.h"
#include "driver/ledc.h"

#include "http_server.h"

static const char *TAG = "HTTP";

extern QueueHandle_t xQueueHttp;

#if 0
#define STORAGE_NAMESPACE "storage"

esp_err_t save_key_value(char * key, char * value)
{
	nvs_handle_t my_handle;
	esp_err_t err;

	// Open
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) return err;

	// Write
	err = nvs_set_str(my_handle, key, value);
	if (err != ESP_OK) return err;

	// Commit written value.
	// After setting any values, nvs_commit() must be called to ensure changes are written
	// to flash storage. Implementations may write to storage at other times,
	// but this is not guaranteed.
	err = nvs_commit(my_handle);
	if (err != ESP_OK) return err;

	// Close
	nvs_close(my_handle);
	return ESP_OK;
}

esp_err_t load_key_value(char * key, char * value, size_t size)
{
	nvs_handle_t my_handle;
	esp_err_t err;

	// Open
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	if (err != ESP_OK) return err;

	// Read
	size_t _size = size;
	err = nvs_get_str(my_handle, key, value, &_size);
	ESP_LOGI(TAG, "nvs_get_str err=%d", err);
	//if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
	if (err != ESP_OK) return err;
	ESP_LOGI(TAG, "err=%d key=[%s] value=[%s] _size=%d", err, key, value, _size);

	// Close
	nvs_close(my_handle);
	//return ESP_OK;
	return err;
}
#endif

int find_key_value(char * key, char * parameter, char * value) 
{
	//char * addr1;
	char * addr1 = strstr(parameter, key);
	if (addr1 == NULL) return 0;
	ESP_LOGD(TAG, "addr1=%s", addr1);

	char * addr2 = addr1 + strlen(key);
	ESP_LOGD(TAG, "addr2=[%s]", addr2);

	char * addr3 = strstr(addr2, "&");
	ESP_LOGD(TAG, "addr3=%p", addr3);
	if (addr3 == NULL) {
		strcpy(value, addr2);
	} else {
		int length = addr3-addr2;
		ESP_LOGD(TAG, "addr2=%p addr3=%p length=%d", addr2, addr3, length);
		strncpy(value, addr2, length);
		value[length] = 0;
	}
	ESP_LOGI(TAG, "key=[%s] value=[%s]", key, value);
	return strlen(value);
}

static esp_err_t Text2Html(httpd_req_t *req, char * filename) {
	ESP_LOGI(TAG, "Reading %s", filename);
	FILE* fhtml = fopen(filename, "r");
	if (fhtml == NULL) {
		ESP_LOGE(TAG, "fopen fail. [%s]", filename);
		return ESP_FAIL;
	} else {
		char line[128];
		while (fgets(line, sizeof(line), fhtml) != NULL) {
			size_t linelen = strlen(line);
			//remove EOL (CR or LF)
			for (int i=linelen;i>0;i--) {
				if (line[i-1] == 0x0a) {
					line[i-1] = 0;
				} else if (line[i-1] == 0x0d) {
					line[i-1] = 0;
				} else {
					break;
				}
			}
			ESP_LOGD(TAG, "line=[%s]", line);
			if (strlen(line) == 0) continue;
			esp_err_t ret = httpd_resp_sendstr_chunk(req, line);
			if (ret != ESP_OK) {
				ESP_LOGE(TAG, "httpd_resp_sendstr_chunk fail %d", ret);
			}
		}
		fclose(fhtml);
	}
	return ESP_OK;
}

esp_err_t Image2Html(httpd_req_t *req, char * filename, char * type)
{
	FILE * fhtml = fopen(filename, "r");
	if (fhtml == NULL) {
		ESP_LOGE(TAG, "fopen fail. [%s]", filename);
		return ESP_FAIL;
	}else{
		char	buffer[64];

		if (strcmp(type, "jpeg") == 0) {
			httpd_resp_sendstr_chunk(req, "<img src=\"data:image/jpeg;base64,");
		} else if (strcmp(type, "jpg") == 0) {
			httpd_resp_sendstr_chunk(req, "<img src=\"data:image/jpeg;base64,");
		} else if (strcmp(type, "png") == 0) {
			httpd_resp_sendstr_chunk(req, "<img src=\"data:image/png;base64,");
		} else {
			ESP_LOGW(TAG, "file type fail. [%s]", type);
			httpd_resp_sendstr_chunk(req, "<img src=\"data:image/png;base64,");
		}
		while(1) {
			size_t bufferSize = fread(buffer, 1, sizeof(buffer), fhtml);
			ESP_LOGD(TAG, "bufferSize=%d", bufferSize);
			if (bufferSize > 0) {
				httpd_resp_send_chunk(req, buffer, bufferSize);
			} else {
				break;
			}
		}
		fclose(fhtml);
		httpd_resp_sendstr_chunk(req, "\">");
	}
	return ESP_OK;
}


/* HTTP get handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "root_get_handler req->uri=[%s]", req->uri);

	/* Send index.html */
	Text2Html(req, "/html/index.html");

	/* Send Image */
	Image2Html(req, "/html/ESP-LOGO.txt", "png");

	/* Send empty chunk to signal HTTP response completion */
	httpd_resp_sendstr_chunk(req, NULL);

	return ESP_OK;
}

/* HTTP post handler */
static esp_err_t root_post_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);
	URL_t urlBuf;
	find_key_value("red=", (char *)req->uri, urlBuf.str_value_red);
	ESP_LOGD(TAG, "urlBuf.str_value_red=[%s]", urlBuf.str_value_red);
	urlBuf.long_value_red = strtol(urlBuf.str_value_red, NULL, 10);
	ESP_LOGD(TAG, "urlBuf.long_value_red=%ld", urlBuf.long_value_red);

	find_key_value("green=", (char *)req->uri, urlBuf.str_value_green);
	ESP_LOGD(TAG, "urlBuf.str_value_green=[%s]", urlBuf.str_value_green);
	urlBuf.long_value_green = strtol(urlBuf.str_value_green, NULL, 10);
	ESP_LOGD(TAG, "urlBuf.long_value_green=%ld", urlBuf.long_value_green);

	find_key_value("blue=", (char *)req->uri, urlBuf.str_value_blue);
	ESP_LOGD(TAG, "urlBuf.str_value_blue=[%s]", urlBuf.str_value_blue);
	urlBuf.long_value_blue = strtol(urlBuf.str_value_blue, NULL, 10);
	ESP_LOGD(TAG, "urlBuf.long_value_blue=%ld", urlBuf.long_value_blue);

	// Send to http_server_task
	if (xQueueSend(xQueueHttp, &urlBuf, portMAX_DELAY) != pdPASS) {
		ESP_LOGE(TAG, "xQueueSend Fail");
	}

	/* Redirect onto root to see the updated file list */
	httpd_resp_set_status(req, "303 See Other");
	httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
	httpd_resp_set_hdr(req, "Connection", "close");
#endif
	httpd_resp_sendstr(req, "post successfully");
	return ESP_OK;
}

/* favicon get handler */
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon_get_handler req->uri=[%s]", req->uri);
	return ESP_OK;
}

/* Function to start the web server */
esp_err_t start_server(const char *base_path, int port)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.server_port = port;

	/* Use the URI wildcard matching function in order to
	 * allow the same handler to respond to multiple different
	 * target URIs which match the wildcard scheme */
	config.uri_match_fn = httpd_uri_match_wildcard;

	ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) != ESP_OK) {
		ESP_LOGE(TAG, "Failed to start file server!");
		return ESP_FAIL;
	}

	/* URI handler for get */
	httpd_uri_t _root_get_handler = {
		.uri		 = "/",
		.method		 = HTTP_GET,
		.handler	 = root_get_handler,
		//.user_ctx  = server_data	// Pass server data as context
	};
	httpd_register_uri_handler(server, &_root_get_handler);

	/* URI handler for post */
	httpd_uri_t _root_post_handler = {
		.uri		 = "/post",
		.method		 = HTTP_POST,
		.handler	 = root_post_handler,
		//.user_ctx  = server_data	// Pass server data as context
	};
	httpd_register_uri_handler(server, &_root_post_handler);

	/* URI handler for favicon.ico */
	httpd_uri_t _favicon_get_handler = {
		.uri		 = "/favicon.ico",
		.method		 = HTTP_GET,
		.handler	 = favicon_get_handler,
		//.user_ctx  = server_data	// Pass server data as context
	};
	httpd_register_uri_handler(server, &_favicon_get_handler);

	return ESP_OK;
}

// LEDC Stuff
#define LEDC_TIMER			LEDC_TIMER_0
#define LEDC_MODE			LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES		LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY			(4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY		(5000) // Frequency in Hertz. Set frequency at 5 kHz

static void ledc_init(void)
{
	// Prepare and then apply the LEDC PWM timer configuration
	ledc_timer_config_t ledc_timer = {
		.speed_mode			= LEDC_MODE,
		.timer_num			= LEDC_TIMER,
		.duty_resolution	= LEDC_DUTY_RES,
		.freq_hz			= LEDC_FREQUENCY,  // Set output frequency at 5 kHz
		.clk_cfg			= LEDC_AUTO_CLK
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	// Prepare and then apply the LEDC PWM channel configuration
	ledc_channel_config_t ledc_channel_red = {
		.speed_mode			= LEDC_MODE,
		.channel			= LEDC_CHANNEL_0,
		.timer_sel			= LEDC_TIMER,
		.intr_type			= LEDC_INTR_DISABLE,
		.gpio_num			= CONFIG_GPIO_RED,
		.duty				= 0, // Set duty to 0%
		.hpoint				= 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_red));

	ledc_channel_config_t ledc_channel_green = {
		.speed_mode			= LEDC_MODE,
		.channel			= LEDC_CHANNEL_1,
		.timer_sel			= LEDC_TIMER,
		.intr_type			= LEDC_INTR_DISABLE,
		.gpio_num			= CONFIG_GPIO_GREEN,
		.duty				= 0, // Set duty to 0%
		.hpoint				= 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_green));

	ledc_channel_config_t ledc_channel_blue = {
		.speed_mode			= LEDC_MODE,
		.channel			= LEDC_CHANNEL_2,
		.timer_sel			= LEDC_TIMER,
		.intr_type			= LEDC_INTR_DISABLE,
		.gpio_num			= CONFIG_GPIO_BLUE,
		.duty				= 0, // Set duty to 0%
		.hpoint				= 0
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_blue));
}

void http_server_task(void *pvParameters)
{
	char *task_parameter = (char *)pvParameters;
	ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter);
	char url[64];
	sprintf(url, "http://%s:%d", task_parameter, CONFIG_WEB_PORT);

	// Set the LEDC peripheral configuration
	ledc_init();

	// Set duty to 50%
	double maxduty = pow(2, 13) - 1;
	float percent_red = 0.5;
	uint32_t duty_red = maxduty * percent_red;
	ESP_LOGI(TAG, "duty_red=%d", duty_red);
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty_red));
	// Update duty to apply the new value
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));

	float percent_green = 0.5;
	uint32_t duty_green = maxduty * percent_green;
	ESP_LOGI(TAG, "duty_green=%d", duty_green);
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty_green));
	// Update duty to apply the new value
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1));

	float percent_blue = 0.5;
	uint32_t duty_blue = maxduty * percent_blue;
	ESP_LOGI(TAG, "duty_blue=%d", duty_blue);
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty_blue));
	// Update duty to apply the new value
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2));

	// Start Server
	ESP_LOGI(TAG, "Starting server on %s", url);
	ESP_ERROR_CHECK(start_server("/spiffs", CONFIG_WEB_PORT));
	
	URL_t urlBuf;
	while(1) {
		// Waiting for post
		if (xQueueReceive(xQueueHttp, &urlBuf, portMAX_DELAY) == pdTRUE) {
			ESP_LOGI(TAG, "str_value_red=%s long_value_red=%ld", urlBuf.str_value_red, urlBuf.long_value_red);
			ESP_LOGI(TAG, "str_value_green=%s long_value_green=%ld", urlBuf.str_value_green, urlBuf.long_value_green);
			ESP_LOGI(TAG, "str_value_blue=%s long_value_blue=%ld", urlBuf.str_value_blue, urlBuf.long_value_blue);

			// Set duty value for red
			percent_red = urlBuf.long_value_red / 100.0;
			duty_red = maxduty * percent_red;
			ESP_LOGI(TAG, "percent_red=%f duty_red=%d", percent_red, duty_red);
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, duty_red));
			// Update duty to apply the new value
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0));

			// Set duty value for green
			percent_green = urlBuf.long_value_green / 100.0;
			duty_green = maxduty * percent_green;
			ESP_LOGI(TAG, "percent_green=%f duty_green=%d", percent_green, duty_green);
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, duty_green));
			// Update duty to apply the new value
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1));

			// Set duty value for blue
			percent_blue = urlBuf.long_value_blue / 100.0;
			duty_blue = maxduty * percent_blue;
			ESP_LOGI(TAG, "percent_blue=%f duty_blue=%d", percent_blue, duty_blue);
			ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_2, duty_blue));
			// Update duty to apply the new value
			ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_2));
		}
	}

	// Never reach here
	ESP_LOGI(TAG, "finish");
	vTaskDelete(NULL);
}
