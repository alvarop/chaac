// Chaac Cloudflare Worker
// Serves the landing page and redirects legacy path-based URLs to per-station subdomains.
// All station pages and APIs are served by Flask on each RPi via Cloudflare Tunnels.

export default {
  async fetch(request, env) {
    const url = new URL(request.url);
    const path = url.pathname;
    const hosts = JSON.parse(env.HOSTS);

    // Root: landing page
    if (path === '/' || path === '') {
      return env.ASSETS.fetch(new Request(new URL('/index.html', url.origin)));
    }

    // Legacy /<host>[/<subpath>] -> 301 to https://<tunnelHost>/<subpath>
    const parts = path.split('/').filter(Boolean);
    const host = parts[0];
    if (host in hosts) {
      const subpath = parts.slice(1).join('/');
      const target = `https://${hosts[host]}/${subpath}${url.search}`;
      return Response.redirect(target, 301);
    }

    return new Response('Not found', { status: 404 });
  },
};
