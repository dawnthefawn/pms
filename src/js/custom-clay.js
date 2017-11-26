module.exports = function(minified){
  var clayConfig = this;
  var _ = minified._;
  var $ = minified.$;
  var HTML = minified.HTML;

  clayConfig.getItemByMessageKey('SERVER_URL').enable();
  clayConfig.getItemByMessageKey('SONARR_PORT').enable();
  clayConfig.getItemByMessageKey('SONARR_API').enable;

}
