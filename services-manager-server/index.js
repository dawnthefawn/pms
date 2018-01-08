const Express = require('express');
const ServicesManagerServer = Express();

ServicesManagerServer.get('/sonarr/restart', (request, response) => response.send('Restart Sonarr!'));
ServicesManagerServer.get('/radarr/restart', (request, response) => response.send('Restart Radarr!'));

ServicesManagerServer.listen(9090, () => console.log('ServicesManagerServer listening on port 9090'));


