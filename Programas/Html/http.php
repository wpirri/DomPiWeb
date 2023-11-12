<?php

function httpRequest($req_typ, $url, $auth, $data)
{
    //$url = 'http://server.com/path';
    //$data = array('key1' => 'value1', 'key2' => 'value2');
    
    // use key 'http' even if you send the request to https://...
    $options = array();
    $http = array();
    $ssl = array();

    $http['header'] = "Content-type: application/x-www-form-urlencoded\r\n";
    if( strlen($auth) > 3 )
    {
        $http['header'] = $http['header']."Authorization: Basic ".base64_encode($auth)."\r\n";
    }
    $http['method'] = $req_typ;
    if( $req_typ == 'POST' )
    {
        $http['content'] = http_build_query($data);
    }
    $ssl['verify_peer'] = false;
    $ssl['verify_peer_name'] = false;

    $options['http'] = $http;
    $options['ssl'] = $ssl;

    $context  = stream_context_create($options);
    $result = file_get_contents($url, false, $context);
    return $result;
}

function httpPost($url, $data, $auth)
{
    return httpRequest('POST', $url, '', $data, $auth);
}

function httpGet($url)
{
    return httpRequest('GET', $url, '', '', $auth);
}

?>
