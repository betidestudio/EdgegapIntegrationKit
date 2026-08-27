'use client';

import { useState, useCallback } from 'react';
import Image from 'next/image';

export interface BlueprintEmbedProps {
  /** Path to the blueprint screenshot image */
  image: string;
  /** Raw blueprint text (copy from UE and paste here) */
  blueprint: string;
  /** Alt text for the image */
  alt?: string;
  /** Optional title displayed above the image */
  title?: string;
}

export function BlueprintEmbed({
  image,
  blueprint,
  alt = 'Blueprint',
  title,
}: BlueprintEmbedProps) {
  const [copied, setCopied] = useState(false);

  const handleCopy = useCallback(async () => {
    try {
      await navigator.clipboard.writeText(blueprint);
      setCopied(true);
      setTimeout(() => setCopied(false), 2000);
    } catch (err) {
      console.error('Failed to copy blueprint:', err);
    }
  }, [blueprint]);

  return (
    <div className="my-6 overflow-hidden rounded-lg border border-fd-border bg-fd-card">
      {title && (
        <div className="border-b border-fd-border px-4 py-2">
          <span className="text-sm font-medium text-fd-foreground">{title}</span>
        </div>
      )}

      <div className="relative bg-[#1a1a1a] flex justify-center">
        <Image
          src={image}
          alt={alt}
          width={800}
          height={450}
          unoptimized
          style={{
            width: '100%',
            height: 'auto',
          }}
        />
      </div>

      <div className="flex items-center justify-between border-t border-fd-border px-4 py-3 bg-fd-muted/30">
        <span className="text-xs text-fd-muted-foreground">
          Copy and paste into Unreal Engine Blueprint editor
        </span>

        <button
          onClick={handleCopy}
          className="inline-flex items-center gap-2 rounded-md bg-fd-primary px-3 py-1.5 text-sm font-medium text-fd-primary-foreground transition-colors hover:bg-fd-primary/90 focus:outline-none focus:ring-2 focus:ring-fd-ring focus:ring-offset-2"
        >
          {copied ? (
            <>
              <CheckIcon className="h-4 w-4" />
              Copied!
            </>
          ) : (
            <>
              <CopyIcon className="h-4 w-4" />
              Copy Blueprint
            </>
          )}
        </button>
      </div>
    </div>
  );
}

function CopyIcon({ className }: { className?: string }) {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth={2}
      strokeLinecap="round"
      strokeLinejoin="round"
      className={className}
    >
      <rect x="9" y="9" width="13" height="13" rx="2" ry="2" />
      <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1" />
    </svg>
  );
}

function CheckIcon({ className }: { className?: string }) {
  return (
    <svg
      xmlns="http://www.w3.org/2000/svg"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth={2}
      strokeLinecap="round"
      strokeLinejoin="round"
      className={className}
    >
      <polyline points="20 6 9 17 4 12" />
    </svg>
  );
}
