/* HTTP Server Example

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
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
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "driver/mcpwm_prelude.h"
#else
#include "driver/mcpwm.h"
#endif

#include "http_server.h"

static const char *TAG = "HTTP";

extern QueueHandle_t xQueueHttp;

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
	find_key_value("value=", (char *)req->uri, urlBuf.str_value);
	ESP_LOGD(TAG, "urlBuf.str_value=[%s]", urlBuf.str_value);
	urlBuf.long_value = strtol(urlBuf.str_value, NULL, 10);
	ESP_LOGD(TAG, "urlBuf.long_value=%ld", urlBuf.long_value);

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

// SERVO Stuff
//#define SERVO_MIN_PULSEWIDTH_US (1000) // Minimum pulse width in microsecond
//#define SERVO_MAX_PULSEWIDTH_US (2000) // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE		-90   // Minimum angle
#define SERVO_MAX_DEGREE		90	  // Maximum angle
//#define SERVO_PULSE_GPIO			(18)	 // GPIO connects to the PWM signal line
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
//#define SERVO_TIMEBASE_PERIOD		 20000	  // 20000 ticks, 20ms

static inline uint32_t convert_servo_angle_to_duty_us(int angle)
{
	return (angle + SERVO_MAX_DEGREE) * (CONFIG_SERVO_MAX_PULSEWIDTH_US - CONFIG_SERVO_MIN_PULSEWIDTH_US) / (2 * SERVO_MAX_DEGREE) + CONFIG_SERVO_MIN_PULSEWIDTH_US;
}

static inline uint32_t convert_servo_angle_to_compare(int angle)
{
	return (angle - SERVO_MIN_DEGREE) * (CONFIG_SERVO_MAX_PULSEWIDTH_US - CONFIG_SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + CONFIG_SERVO_MIN_PULSEWIDTH_US;
}

void http_server_task(void *pvParameters)
{
	char *task_parameter = (char *)pvParameters;
	ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter);
	char url[64];
	sprintf(url, "http://%s:%d", task_parameter, CONFIG_WEB_PORT);

	// Setup Servo
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	ESP_LOGI(TAG, "Create timer and operator");
	// Servo PWM Period = 50Hz, i.e. for every servo motor time period should be 20ms
	uint32_t period_ticks = SERVO_TIMEBASE_RESOLUTION_HZ/CONFIG_SERVO_PWM_PERIOD;
	ESP_LOGI(TAG, "period_ticks=%"PRIu32, period_ticks);
	mcpwm_timer_handle_t timer = NULL;
	mcpwm_timer_config_t timer_config = {
		.group_id = 0,
		.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
		.resolution_hz = SERVO_TIMEBASE_RESOLUTION_HZ,
		//.period_ticks = SERVO_TIMEBASE_PERIOD,
		.period_ticks = period_ticks,
		.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
	};
	ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

	mcpwm_oper_handle_t oper = NULL;
	mcpwm_operator_config_t operator_config = {
		.group_id = 0, // operator must be in the same group to the timer
	};
	ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

	ESP_LOGI(TAG, "Connect timer and operator");
	ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

	ESP_LOGI(TAG, "Create comparator and generator from the operator");
	mcpwm_cmpr_handle_t comparator = NULL;
	mcpwm_comparator_config_t comparator_config = {
		.flags.update_cmp_on_tez = true,
	};
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

	mcpwm_gen_handle_t generator = NULL;
	mcpwm_generator_config_t generator_config = {
		.gen_gpio_num = CONFIG_SERVO_PULSE_GPIO,
	};
	ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

	// set the initial compare value, so that the servo will spin to the center position
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, convert_servo_angle_to_compare(0)));

	ESP_LOGI(TAG, "Set generator action on timer and compare event");
	// go high on counter empty
	ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(generator,
					MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
					MCPWM_GEN_TIMER_EVENT_ACTION_END()));
	// go low on compare threshold
	ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(generator,
					MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW),
					MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

	ESP_LOGI(TAG, "Enable and start timer");
	ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
	ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));

#else
	mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, CONFIG_SERVO_PULSE_GPIO); // To drive a RC servo, one MCPWM generator is enough

	mcpwm_config_t pwm_config = {
		.frequency = CONFIG_SERVO_PWM_PERIOD, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
		.cmpr_a = 0, // duty cycle of PWMxA = 0
		.counter_mode = MCPWM_UP_COUNTER,
		.duty_mode = MCPWM_DUTY_MODE_0,
	};
	mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
#endif

	// Start Server
	ESP_LOGI(TAG, "Starting server on %s", url);
	ESP_ERROR_CHECK(start_server("/spiffs", CONFIG_WEB_PORT));
	
	// Rotate to the center
	int angle = 0;
	ESP_LOGI(TAG, "Angle of rotation: %d", angle);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, convert_servo_angle_to_compare(angle)));
#else
	ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(angle)));
#endif

	URL_t urlBuf;
	while(1) {
		// Waiting for post
		if (xQueueReceive(xQueueHttp, &urlBuf, portMAX_DELAY) == pdTRUE) {
			ESP_LOGI(TAG, "str_value=%s long_value=%ld", urlBuf.str_value, urlBuf.long_value);
			
			angle = urlBuf.long_value;
			ESP_LOGI(TAG, "Angle of rotation: %d", angle);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
			ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, convert_servo_angle_to_compare(angle)));
#else
			ESP_ERROR_CHECK(mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, convert_servo_angle_to_duty_us(angle)));
#endif

		}
	}

	// Never reach here
	ESP_LOGI(TAG, "finish");
	vTaskDelete(NULL);
}
