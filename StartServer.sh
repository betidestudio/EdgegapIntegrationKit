#!/bin/sh

GAME_PORT=$(echo $ARBITRIUM_PORTS_MAPPING | jq '.ports.gameport.internal')

printenv
env

# Function to call Edgegap self-stop API
call_stop_api() {
    local EXIT_CODE=$1
    
    # Use production self-stop endpoint (ARBITRIUM_DELETE_URL) - required for production
    local DELETE_URL="${ARBITRIUM_DELETE_URL}"
    local DELETE_TOKEN="${ARBITRIUM_DELETE_TOKEN}"
    
    if [ -z "$DELETE_URL" ] || [ -z "$DELETE_TOKEN" ]; then
        echo "Error: Production self-stop method not available - missing ARBITRIUM_DELETE_URL or ARBITRIUM_DELETE_TOKEN"
        echo "Note: The C++ subsystem may still call the self-stop API if it has access to the environment variables"
        exit $EXIT_CODE
    fi
    
    # Production method - use deployment-specific URL and token
    echo "Calling Edgegap production self-stop API"
    
    RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE \
        -H "Content-Type: application/json" \
        -H "Authorization: $DELETE_TOKEN" \
        "$DELETE_URL" \
        --max-time 5)
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -ge 200 ] && [ "$HTTP_CODE" -lt 300 ]; then
        echo "Successfully called production self-stop API (HTTP $HTTP_CODE)"
    else
        echo "Production self-stop API call returned HTTP $HTTP_CODE: $BODY"
    fi
    
    exit $EXIT_CODE
}

# Trap exit signals to call stop API
trap 'call_stop_api $?' EXIT INT TERM

# Run the server
$(dirname "$0")/<PROJECT_NAME>Server.sh -log -PORT=$GAME_PORT
SERVER_EXIT_CODE=$?

# If server exits normally, call stop API with its exit code
call_stop_api $SERVER_EXIT_CODE