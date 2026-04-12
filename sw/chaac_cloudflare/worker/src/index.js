// Chaac Cloudflare Worker
// Routes requests to RPi weather stations via Cloudflare Tunnels
// Caches JSON API responses at the edge

const CACHE_TTL = 60; // seconds

// Static pages served for these path patterns
const PAGE_ROUTES = {
  'plots': '/plots.html',
  'stats': '/stats.html',
};

// API paths that get proxied to RPi tunnels
const API_PATHS = ['latest', 'json/', 'zipdb'];

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const path = url.pathname;
    const hosts = JSON.parse(env.HOSTS);

    // Root: serve landing page
    if (path === '/' || path === '') {
      return env.ASSETS.fetch(new Request(new URL('/index.html', url.origin)));
    }

    // Serve static assets (layout.js, etc.)
    if (path === '/layout.js' || path.startsWith('/static/')) {
      return env.ASSETS.fetch(request);
    }

    // Parse /<host> or /<host>/<path>
    const parts = path.split('/').filter(Boolean);
    if (parts.length === 0) {
      return new Response('Not found', { status: 404 });
    }

    const host = parts[0];
    const subpath = parts.slice(1).join('/');

    // Validate host
    if (!(host in hosts)) {
      return new Response('Not found', { status: 404 });
    }

    // /<host> -> serve status page
    if (parts.length === 1) {
      return env.ASSETS.fetch(new Request(new URL('/status.html', url.origin)));
    }

    // /<host>/plots or /<host>/stats -> serve static page
    if (subpath in PAGE_ROUTES) {
      return env.ASSETS.fetch(new Request(new URL(PAGE_ROUTES[subpath], url.origin)));
    }

    // API requests: proxy to RPi tunnel with caching
    const isApiPath = API_PATHS.some(p => subpath.startsWith(p));
    if (!isApiPath) {
      return new Response('Not found', { status: 404 });
    }

    // Check cache first
    const cache = caches.default;
    const cacheKey = new Request(url.toString(), request);
    let response = await cache.match(cacheKey);
    if (response) {
      return response;
    }

    // Fetch from RPi via tunnel
    const tunnelHost = hosts[host];
    const tunnelUrl = `https://${tunnelHost}/${subpath}`;

    try {
      const originResponse = await fetch(tunnelUrl, {
        headers: {
          'Accept': 'application/json',
        },
      });

      if (!originResponse.ok) {
        return new Response(
          JSON.stringify({ error: `Upstream error (${originResponse.status})` }),
          { status: originResponse.status, headers: { 'Content-Type': 'application/json' } }
        );
      }

      // Build cacheable response
      response = new Response(originResponse.body, originResponse);
      response.headers.set('Cache-Control', `public, max-age=${CACHE_TTL}`);
      response.headers.set('Access-Control-Allow-Origin', '*');

      // Don't cache zipdb (large file downloads)
      if (!subpath.startsWith('zipdb')) {
        await cache.put(cacheKey, response.clone());
      }

      return response;
    } catch (err) {
      return new Response(
        JSON.stringify({ error: 'Error connecting to weather station' }),
        { status: 502, headers: { 'Content-Type': 'application/json' } }
      );
    }
  },

  // Optional: Cron trigger to pre-warm cache
  // Uncomment [triggers] in wrangler.toml to enable
  async scheduled(event, env, ctx) {
    const hosts = JSON.parse(env.HOSTS);
    const endpoints = ['latest'];

    for (const [name, tunnelHost] of Object.entries(hosts)) {
      for (const endpoint of endpoints) {
        try {
          const url = `https://${tunnelHost}/${endpoint}`;
          const response = await fetch(url);
          if (response.ok) {
            // Store in cache
            const cache = caches.default;
            const cacheUrl = `https://chaac.network/${name}/${endpoint}`;
            const cacheKey = new Request(cacheUrl);
            const cacheResponse = new Response(response.body, response);
            cacheResponse.headers.set('Cache-Control', `public, max-age=${CACHE_TTL}`);
            await cache.put(cacheKey, cacheResponse);
          }
        } catch (e) {
          // Station offline, skip
        }
      }
    }
  },
};
