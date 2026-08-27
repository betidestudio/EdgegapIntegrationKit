<h1 align="center" id="title">Edgegap Integration Kit</h1>

<p align="center"><img src="https://socialify.git.ci/betidestudio/EdgegapIntegrationKit/image?description=1&descriptionEditable=The%20ultimate%20EdgeGap%20Integration%20Kit&font=Inter&language=1&name=1&owner=1&pattern=Plus&stargazers=1&theme=Dark" alt="project-image" width="600" height="338/"></p>

<p id="description">EThe Edgegap Integration Kit simplifies the process of deploying and managing multiplayer game servers using Edgegap’s distributed orchestration platform. By leveraging Edgegap's automated game server deployment across its global edge computing infrastructure, you can ensure low latency, scalable server hosting, and seamless integration with Unreal Engine.</p>

- [Discord Support](https://discord.gg/betidestudio)
- [Mail Support](mailto:support@betide.studio)
- [Unreal Engine Marketplace Version](https://www.unrealengine.com/marketplace/en-US/product/edgegap-integration-kit)

<h2>🚀 Setup Information</h2>

- [Video](https://www.youtube.com)
- [Documentation](https://egik.betide.studio)

<h2>🧐 Features</h2>

Here're some of the project's best features:

*   Regionless Hosting: Automatically cache and deploy game servers across 615+ locations worldwide, ensuring optimal server placement with reduced latency.
*   Cross-Platform Compatibility: Integrates seamlessly with Unreal Engine and other major platforms, supporting a wide range of game types and sizes.
*   Bare Metal & Cloud Integration: Hybrid orchestration enables efficient hosting, combining bare metal servers for low-traffic periods and cloud infrastructure for spikes in demand.
*   Instant DDoS Protection: Built-in defense mechanisms for keeping your servers secure and running smoothly.
*   Fast Boot Times: Game servers deploy and become operational in under 3 seconds, allowing for rapid scaling to meet player demand.
*   Pay-Per-Use Pricing: With no upfront costs, you only pay for server hosting when players are actively using your game.
*   Easy Integration: Simple setup and configuration with the kit, allowing you to focus on developing your game rather than making API calls.
*   Addon Integration: Supports EOS Integration Kit for Epic Online Services and Steam Integration Kit for seamless integration with the Steam platform.

<h2>📝 Changelog</h2>

### Version 2.0 Beta (Latest) - 2026-08-15

#### Matchmaking Improvements:
- Added type-safe Blueprint matchmaking attributes for strings, numbers, booleans, intersection arrays, and beacon latency maps while retaining raw JSON compatibility
- Added local validation for matchmaking URLs, authorization tokens, profiles, resource IDs, group members, and structured attributes
- Added optional non-blocking runtime profile diagnostics that compare ticket, Group Up, and group-ticket structured attributes against the active Matchmaker's generated Swagger schema
- Standardized Matchmaker, Group Up, Group Ticket, and Backfill URL/auth resolution and structured API error reporting
- Added per-attempt timeouts and bounded retries for network failures, rate limits, and transient server errors with exponential jittered backoff and `Retry-After` support
- Updated ticket creation to serialize the supported `player_ip` JSON field and deprecated the legacy `RealIp` input
- Updated server environment helpers to use `MM_GROUPS` and `MM_EXPANSION`, with fallbacks for legacy variable names
- Added Blueprint travel helpers that resolve assignment ports and safely move remote party members before a listen-server leader, with no custom RPC or component setup

#### Backfill Fixes:
- Fixed `CreateBackfill` silently omitting attribute overrides and malformed ticket JSON
- Added strict validation for deployment assignment details, ticket payloads, and reserved assignment fields
- Hardened Create/Get Backfill response parsing for omitted, null, and incorrectly typed JSON fields
- Standardized Backfill create, get, and delete connection handling

#### Compatibility and Documentation:
- Added Unreal Engine 5.8-compatible JSON map-key handling while preserving UE 5.7 and earlier behavior
- Updated Matchmaking, Group Up, deployment environment, and troubleshooting documentation

### Version 1.7 - 2025-04-13

#### Breaking Changes:
- Updated to support Edgegap API v2.1.0 changes
- Added support for team ID in ticket assignments
- Updated Backfill API request structure (moved `attributes.deployment_request_id` to `attributes.assignment.request_id`)
- Added environment variable helpers for MM_MATCH_PROFILE and MM_EXPANSION_STAGE
- Updated ticket deletion handling to support 409 Conflict response code

#### Improvements:
- Added support for additional port configurations in project settings
- Added support for default environment variables in project settings
- Added GetGroupPlayerMapping function for retrieving player IDs by group ID
- Added GetExpansionStage function to get expansion stage information
- Added GetMatchProfileName function to get match profile name
- Added AssignmentDetails field to Backfills creation blueprint for full assignment details support in JSON format
- Fixed Dockerfile modification issue
- Fixed compiler warnings with LOCTEXT_NAMESPACE definitions

<h2>🛠️ Installation Steps:</h2>

Please check the docs for updated Installation Steps

<h2>🍰 Contribution Guidelines:</h2>

Being a open-source project it's important for the people to contribute to the plugin. The only guideline is to include all headers because 5.3 has a lot of issues with headers xD

  
  
<h2>💻 Built with</h2>

Technologies used in the project:

*   Unreal Engine
*   C++

<h2>🛡️ License:</h2>

This project is licensed under the MIT License

<h2>💖Like our work?</h2>

Consider supporting us at [Patreon](https://www.patreon.com/betide)
Hire us at [support@betide.studio](mailto:support@betide.studio)
