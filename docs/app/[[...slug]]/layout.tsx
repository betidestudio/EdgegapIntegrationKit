import { source } from '@/lib/source';
import { DocsLayout } from '@/components/layout/docs';
import { baseOptions } from '@/lib/layout.shared';
import Image from 'next/image';
import Link from 'next/link';

function SidebarFooter() {
  return (
    <Link
      href="https://betide.studio"
      target="_blank"
      className="flex items-center gap-2 text-sm hover:text-fd-foreground transition-colors"
    >
      <Image src="/logo.svg" alt="Betide Studio" width={18} height={18} />
      <span>Betide Studio</span>
    </Link>
  );
}

export default function Layout({ children }: LayoutProps<'/'>) {
  return (
    <DocsLayout
      tree={source.getPageTree()}
      {...baseOptions()}
      sidebar={{ footer: <SidebarFooter /> }}
    >
      {children}
    </DocsLayout>
  );
}
