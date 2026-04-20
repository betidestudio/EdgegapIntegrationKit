#!/bin/bash

pid=$$
EXECUTABLE_PATH="$(dirname "$0")"
: ${TARGET_FILE_NAME:="<TARGET_FILE_NAME>"} # replaced during containerization

resolve_server_launcher() {
  local candidate

  # 1) Explicit target name
  if [ -n "$TARGET_FILE_NAME" ]; then
    if [ -f "$EXECUTABLE_PATH/$TARGET_FILE_NAME.sh" ]; then
      echo "$EXECUTABLE_PATH/$TARGET_FILE_NAME.sh"
      return 0
    fi
    if [ -x "$EXECUTABLE_PATH/$TARGET_FILE_NAME" ]; then
      echo "$EXECUTABLE_PATH/$TARGET_FILE_NAME"
      return 0
    fi
  fi

  # 2) First matching dedicated server launch script
  for candidate in "$EXECUTABLE_PATH"/*Server.sh; do
    if [ -f "$candidate" ]; then
      echo "$candidate"
      return 0
    fi
  done

  # 3) First matching dedicated server binary
  for candidate in "$EXECUTABLE_PATH"/*Server; do
    if [ -x "$candidate" ] && [ "${candidate##*.}" = "${candidate}" ]; then
      echo "$candidate"
      return 0
    fi
  done

  return 1
}

# ============================================
# Signal handlers
# ============================================
separator() {
  printf "%0.s~" {1..80}
  echo ""
}

catch_term() {
  echo "TERM signal received..."
  separator
  kill -TERM "$pid" 2>/dev/null
}

catch_quit() {
  echo "QUIT signal received..."
  separator
  kill -QUIT "$pid" 2>/dev/null
}

trap catch_term TERM
trap catch_quit QUIT

# ============================================
# Print environment for debugging
# ============================================
separator
echo "ARBITRIUM_PORTS_MAPPING:"
echo "$ARBITRIUM_PORTS_MAPPING"

separator
echo "ENVIRONMENT VARS:"
env

# ============================================
# Resolve game port
# ============================================
GAME_PORT="$ARBITRIUM_PORT_GAMEPORT_INTERNAL"
if [ -z "$GAME_PORT" ] && [ -n "$ARBITRIUM_PORTS_MAPPING" ]; then
  # Fallback: parse from JSON mapping if direct env var not available
  GAME_PORT=$(echo "$ARBITRIUM_PORTS_MAPPING" | jq -r '.ports.gameport.internal' 2>/dev/null)
fi

# ============================================
# Start game server
# ============================================
SERVER_LAUNCHER="$(resolve_server_launcher)"
if [ -z "$SERVER_LAUNCHER" ]; then
  separator
  echo "Could not find dedicated server launcher in $EXECUTABLE_PATH"
  echo "TARGET_FILE_NAME=$TARGET_FILE_NAME"
  SERVER_EXIT_CODE=127
else
  SERVER_EXIT_CODE=0
fi

separator
if [ -n "$SERVER_LAUNCHER" ]; then
  if [[ "$SERVER_LAUNCHER" == *.sh ]]; then
    echo "Execute command: /bin/bash $SERVER_LAUNCHER -log -PORT=$GAME_PORT $UE_COMMANDLINE_ARGS"
  else
    echo "Execute command: $SERVER_LAUNCHER -log -PORT=$GAME_PORT $UE_COMMANDLINE_ARGS"
  fi
fi
separator
if [ -n "$SERVER_LAUNCHER" ]; then
  if [[ "$SERVER_LAUNCHER" == *.sh ]]; then
    /bin/bash "$SERVER_LAUNCHER" -log -PORT="$GAME_PORT" $UE_COMMANDLINE_ARGS
  else
    "$SERVER_LAUNCHER" -log -PORT="$GAME_PORT" $UE_COMMANDLINE_ARGS
  fi
  SERVER_EXIT_CODE=$?
fi

# ============================================
# After game server process terminates
# ============================================
separator
echo "Gameserver exit code: $SERVER_EXIT_CODE"

# ============================================
# Call Edgegap self-stop API
# ============================================
separator
if [ -n "$ARBITRIUM_DELETE_URL" ] && [ -n "$ARBITRIUM_DELETE_TOKEN" ]; then
  echo "Calling self-stop API: DELETE $ARBITRIUM_DELETE_URL"
  RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE \
    -H "Authorization: ${ARBITRIUM_DELETE_TOKEN}" \
    "${ARBITRIUM_DELETE_URL}" \
    --max-time 5)

  HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
  BODY=$(echo "$RESPONSE" | sed '$d')

  if [ "$HTTP_CODE" -ge 200 ] 2>/dev/null && [ "$HTTP_CODE" -lt 300 ] 2>/dev/null; then
    echo "Self-stop API success (HTTP $HTTP_CODE)"
  else
    echo "Self-stop API returned HTTP $HTTP_CODE: $BODY"
  fi
elif [ -n "$ARBITRIUM_REQUEST_ID" ]; then
  # Fallback: construct URL manually with API key
  API_KEY="${EDGEGAP_API_KEY:-${EG_API_KEY}}"
  if [ -n "$API_KEY" ]; then
    echo "Calling self-stop API (fallback): DELETE https://api.edgegap.com/v1/stop/$ARBITRIUM_REQUEST_ID"
    RESPONSE=$(curl -s -w "\n%{http_code}" -X DELETE \
      -H "Content-Type: application/json" \
      -H "Authorization: $API_KEY" \
      "https://api.edgegap.com/v1/stop/$ARBITRIUM_REQUEST_ID" \
      --max-time 5)

    HTTP_CODE=$(echo "$RESPONSE" | tail -n1)
    BODY=$(echo "$RESPONSE" | sed '$d')

    if [ "$HTTP_CODE" -ge 200 ] 2>/dev/null && [ "$HTTP_CODE" -lt 300 ] 2>/dev/null; then
      echo "Self-stop API success (HTTP $HTTP_CODE)"
    else
      echo "Self-stop API returned HTTP $HTTP_CODE: $BODY"
    fi
  else
    echo "Have request ID but no ARBITRIUM_DELETE_TOKEN or EDGEGAP_API_KEY — cannot call self-stop"
  fi
else
  echo "No Edgegap environment detected — skipping self-stop API call"
fi

# Keep container alive for Edgegap to collect logs and send SIGTERM.
# Safety timeout: exit after 5 minutes if Edgegap hasn't terminated us.
echo "Waiting for Edgegap to terminate the deployment..."
sleep 300
echo "Safety timeout reached (5 min) — forcing container exit"
exit "$SERVER_EXIT_CODE"
