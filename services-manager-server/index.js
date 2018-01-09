const Express = require('express');
const ServicesManagerServer = Express();
var Shell = require('shelljs');

if (!shell.which('bash'))
{
	shell.echo('Even if you dont use arch linux, bash is a minimum requuirement for this addon.');

	shell.exit(1);
}

var ShellProcess = exec('sh ~/services-server-manager/sh/sonarr.sh')

ServicesManagerServer.get('/sonarr/restart', (request, response) => response.send('Restart Sonarr!'));
ServicesManagerServer.get('/radarr/restart', (request, response) => response.send('Restart Radarr!'));

ServicesManagerServer.listen(9090, () => console.log('ServicesManagerServer listening on port 9090'));


