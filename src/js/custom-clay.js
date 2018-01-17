module.exports = function(minified){
	var clayConfig = this;
	var _ = minified._;
	var $ = minified.$;
	var HTML = minified.HTML;
	var server_base_url;// = Clay.getItemByMessageKey('SERVER_URL'); 
	var sonarr_api_key;// = Clay.getItemByMessageKey('SONARR_API'); 
	var sonarr_port;// = Clay.getItemByMessageKey('SONARR_PORT'); 
	var sonarr_search_string = '/api/series/lookup?term='; 
	var sonarr_postfix;// = '&apikey=' + sonarr_api_key; 
	var pms_service; 
	var pms_request_type; 
	var pms_request_url; 
	var pms_request; 
	var request_type_string; 
	var method; 
	var request = new XMLHttpRequest();

	clayConfig.getItemByMessageKey('SERVER_URL').enable();
	clayConfig.getItemByMessageKey('SONARR_PORT').enable();
	clayConfig.getItemByMessageKey('SONARR_API').enable;

	clayConfig.getItemByMessageKey('RADARR_PORT').enable;

	clayConfig.getItemByMessageKey('RADARR_API').enable;
	clayConfig.getItemByMessageKey('SMS_PORT').enable;
}
