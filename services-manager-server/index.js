const Express = require('express');
const ServicesManagerServer = Express();
var Shell = require('shelljs');

if (!Shell.which('bash'))
{
	Shell.echo('Even if you dont use arch linux, bash is a minimum requuirement for this addon.');

	Shell.exit(1);
}


ServicesManagerServer.get('/sonarr/restart', (request, response) =>
{
		
	if(Shell.exec('sh ~/services-manager-server/sh/sonarr.sh'))
	{
		response.send({"status": "Restarted Sonarr!"});
	}
	else
	{
		response.send({"status": "ERROR: FAILED"});
	}
});


ServicesManagerServer.get('/radarr/restart', (request, response) =>
{
	if (Shell.exec('sh ~/services-manager-server/sh/radarr.sh'))
	{
		response.send({"status": "Restarted Radarr!"});
	}
	else
	{
		response.send({"status": "ERROR: FAILED"});
	}
});

ServicesManagerServer.listen(9090, () => console.log('ServicesManagerServer listening on port 9090'));


