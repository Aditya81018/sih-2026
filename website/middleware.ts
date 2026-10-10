import { NextResponse } from "next/server";
import type { NextRequest } from "next/server";

export function middleware(request: NextRequest) {
  const { pathname, searchParams } = request.nextUrl;

  if (pathname === "/cost-list") {
    const acceptHeader = request.headers.get("accept") || "";
    const format = searchParams.get("format");

    // If client explicitly requested JSON or CSV via query param or Accept header
    const wantsJson = format === "json" || (acceptHeader.includes("application/json") && !acceptHeader.includes("text/html"));
    const wantsCsv = format === "csv" || acceptHeader.includes("text/csv");

    if (wantsJson || wantsCsv) {
      const url = request.nextUrl.clone();
      url.pathname = "/api/cost-list";
      return NextResponse.rewrite(url);
    }
  }

  return NextResponse.next();
}

export const config = {
  matcher: ["/cost-list"],
};
