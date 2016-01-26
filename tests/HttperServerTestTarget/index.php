<?php

/*
 * Easiest way to start this test server is to use the build in web server in PHP.
 * To do this, execute the following command from the ROOT directory of this script
 *
 * $ php -S localhost:8888
 *
 * After issuing the above command, open up Httper and enter the url http://localhost:8888 and execute the request!
 *
 */

date_default_timezone_set("UTC");

require_once(__DIR__ . '/vendor/autoload.php');

use Symfony\Component\HttpFoundation\Request;
use Symfony\Component\HttpFoundation\Response;

$app = new Silex\Application();

// Auto decode JSON from POST/PUT requests
$app->before(function(Request $request) {
    if (0 === strpos($request->headers->get("Content-Type"), "application/json")) {
        $data = json_decode($request->getContent(), TRUE, 50);
        $request->request->replace(is_array($data) ? $data : Array());
    }
});

$app->get('/', function() use ($app) {
    return new Response("This is the Httper test server root URL", 200, Array(
        "X-Httper-TestServer" => "true; v. 0.11"
    ));
});

$app->post('/test/json', function(Request $request) use($app) {
    $inJson = json_decode($request->getContent(), TRUE, 50);
    $headers = $request->headers->all();

    return $app->json(Array(
        "request_json" => $inJson,
        "headers" => $headers
    ));
});

$app->run();
