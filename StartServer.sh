#!/bin/sh

GAME_PORT=$(echo $ARBITRIUM_PORTS_MAPPING | jq '.ports.gameport.internal')

printenv
env

# Function to call Edgegap self-stop API
call_stop_api() {
    local EXIT_CODE=$1
    # Try multiple possible environment variable names for request ID
    local REQUEST_ID="${EG_REQUEST_ID:-${EDGEGAP_REQUEST_ID:-${EDGE_REQUEST_ID:-${ARBITRIUM_REQUEST_ID}}}}"
    
    # API key may not be available as env var (typically stored in config)
    # Try environment first, then common config locations
    local API_KEY="${EG_API_KEY:-${EDGEGAP_API_KEY:-${EDGE_API_KEY}}}"
    
    # If API key not in env, try reading from config file (if available)
    if [ -z "$API_KEY" ] && [ -f "${UE_PROJECT_DIR}/Saved/Config/LinuxServer/DefaultEngine.ini" ]; then
        API_KEY=$(grep -m 1 "^AuthorizationKey=" "${UE_PROJECT_DIR}/Saved/Config/LinuxServer/DefaultEngine.ini" 2>/dev/null | cut -d'=' -f2 | tr -d ' ')
    fi
    
    if [ -z "$REQUEST_ID" ]; then
        echo "No Edgegap request ID found in environment variables (checked: EG_REQUEST_ID, EDGEGAP_REQUEST_ID, EDGE_REQUEST_ID, ARBITRIUM_REQUEST_ID)"
        echo "Note: The C++ subsystem may still call the self-stop API if it has access to the request ID"
        exit $EXIT_CODE
    fi
    
    if [ -z "$API_KEY" ]; then
        echo "No Edgegap API key found (checked env vars and config file)"
        echo "Note: The C++ subsystem may still call the self-stop API if it has access to the API key from config"
        exit $EXIT_CODE
    fi
    
    echo "Calling Edgegap self-stop API for request ID: $REQUEST_ID"
    
    # Call the self-stop API
    RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE \
        -H "Content-Type: application/json" \
        -H "Authorization: $API_KEY" \
        "https://api.edgegap.com/v1/stop/$REQUEST_ID" \
        --max-time 5)
    
    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')
    
    if [ "$HTTP_CODE" -ge 200 ] && [ "$HTTP_CODE" -lt 300 ]; then
        echo "Successfully called self-stop API (HTTP $HTTP_CODE)"
    else
        echo "Self-stop API call returned HTTP $HTTP_CODE: $BODY"
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