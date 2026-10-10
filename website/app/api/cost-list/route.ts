import { NextRequest, NextResponse } from "next/server";
import { COST_ITEMS, TOTAL_COST, TOTAL_COST_FORMATTED, RAW_CSV, CATEGORY_SUMMARIES } from "@/app/cost-list/data";

export async function GET(request: NextRequest) {
  const searchParams = request.nextUrl.searchParams;
  const format = searchParams.get("format")?.toLowerCase();
  const category = searchParams.get("category");
  const query = searchParams.get("search")?.toLowerCase();

  // If CSV format is requested
  if (format === "csv") {
    return new NextResponse(RAW_CSV, {
      status: 200,
      headers: {
        "Content-Type": "text/csv; charset=utf-8",
        "Content-Disposition": 'inline; filename="mast_subsystem_cost_list.csv"',
        "Cache-Control": "public, max-age=3600, s-maxage=3600",
      },
    });
  }

  // Filter items if params provided
  let filteredItems = [...COST_ITEMS];

  if (category) {
    filteredItems = filteredItems.filter(
      (item) => item.category.toLowerCase() === category.toLowerCase()
    );
  }

  if (query) {
    filteredItems = filteredItems.filter(
      (item) =>
        item.subsystemAxis.toLowerCase().includes(query) ||
        item.componentModel.toLowerCase().includes(query) ||
        item.badge.toLowerCase().includes(query)
    );
  }

  const filteredTotal = filteredItems.reduce((acc, item) => acc + item.totalCost, 0);

  return NextResponse.json(
    {
      project: "Smart India Hackathon 2026",
      subsystem: "Primary Mast & Telescopic Arm Subsystem",
      currency: "INR",
      currencySymbol: "₹",
      totalCost: TOTAL_COST,
      totalCostFormatted: TOTAL_COST_FORMATTED,
      totalItemsCount: COST_ITEMS.length,
      filteredCount: filteredItems.length,
      filteredTotalCost: filteredTotal,
      filteredTotalCostFormatted: `₹${filteredTotal.toLocaleString("en-IN")}`,
      categorySummaries: CATEGORY_SUMMARIES,
      items: filteredItems,
      rawCsv: RAW_CSV,
    },
    {
      status: 200,
      headers: {
        "Content-Type": "application/json; charset=utf-8",
        "Cache-Control": "public, max-age=3600, s-maxage=3600",
        "Access-Control-Allow-Origin": "*",
        "Access-Control-Allow-Methods": "GET, OPTIONS",
      },
    }
  );
}

export async function OPTIONS() {
  return new NextResponse(null, {
    status: 204,
    headers: {
      "Access-Control-Allow-Origin": "*",
      "Access-Control-Allow-Methods": "GET, OPTIONS",
      "Access-Control-Allow-Headers": "Content-Type, Authorization",
    },
  });
}
