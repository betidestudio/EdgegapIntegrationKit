'use client';

import { useState, useEffect } from 'react';
import { X } from 'lucide-react';

const BANNER_DISMISSED_KEY = 'egik-v2-banner-dismissed';

export function VersionBanner() {
  const [dismissed, setDismissed] = useState(true);

  useEffect(() => {
    const isDismissed = localStorage.getItem(BANNER_DISMISSED_KEY) === 'true';
    setDismissed(isDismissed);
  }, []);

  const handleDismiss = () => {
    localStorage.setItem(BANNER_DISMISSED_KEY, 'true');
    setDismissed(true);
  };

  if (dismissed) return null;

  return (
    <div className="sticky top-0 z-50 bg-amber-500/90 dark:bg-amber-600/90 text-black px-4 py-2 text-sm text-center">
      <span>
        This is documentation for <strong>EGIK v2 (Beta)</strong>.
        For the stable version, see{' '}
        <a
          href="https://egikv1.betide.studio"
          className="underline font-medium hover:no-underline"
          target="_blank"
          rel="noopener noreferrer"
        >
          v1 docs
        </a>
        . Try the beta via{' '}
        <a
          href="https://betide.studio"
          className="underline font-medium hover:no-underline"
          target="_blank"
          rel="noopener noreferrer"
        >
          our website
        </a>
        {' '}or join{' '}
        <a
          href="https://discord.gg/Fcj68FJzAj"
          className="underline font-medium hover:no-underline"
          target="_blank"
          rel="noopener noreferrer"
        >
          Discord
        </a>
        .
      </span>
      <button
        onClick={handleDismiss}
        className="absolute right-2 top-1/2 -translate-y-1/2 p-1 hover:bg-black/10 rounded"
        aria-label="Dismiss banner"
      >
        <X className="w-4 h-4" />
      </button>
    </div>
  );
}
