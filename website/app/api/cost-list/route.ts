import { NextRequest, NextResponse } from "next/server";
import {
  COST_ITEMS,
  TOTAL_FACTORY_COST,
  TOTAL_FACTORY_COST_FORMATTED,
  PRICING_MODEL,
  MODULE_SUMMARIES,
  RAW_CSV,
} from "@/app/cost-list/data";

export async function GET(request: NextRequest) {
  const searchParams = request.nextUrl.searchParams;
  const format = searchParams.get("format")?.toLowerCase();
  const moduleParam = searchParams.get("module");
  const query = searchParams.get("search")?.toLowerCase();

  // If CSV format is requested
  if (format === "csv") {
    return new NextResponse(RAW_CSV, {
      status: 200,
      headers: {
        "Content-Type": "text/csv; charset=utf-8",
        "Content-Disposition": 'inline; filename="orbit_v1_cost_breakdown.csv"',
        "Cache-Control": "public, max-age=3600, s-maxage=3600",
      },
    });
  }

  // Filter items if params provided
  let filteredItems = [...COST_ITEMS];

  if (moduleParam) {
    filteredItems = filteredItems.filter(
      (item) => item.systemModule.toLowerCase() === moduleParam.toLowerCase()
    );
  }

  if (query) {
    filteredItems = filteredItems.filter(
      (item) =>
        item.systemModule.toLowerCase().includes(query) ||
        item.itemDescription.toLowerCase().includes(query) ||
        (item.specification && item.specification.toLowerCase().includes(query))
    );
  }

  const filteredTotal = filteredItems.reduce((acc, item) => acc + item.subtotal, 0);

  return NextResponse.json(
    {
      project: "Smart India Hackathon 2026",
      system: "Orbit V1 AGV + Telescopic Mast System",
      currency: "INR",
      currencySymbol: "₹",
      totalFactoryCost: TOTAL_FACTORY_COST,
      totalFactoryCostFormatted: TOTAL_FACTORY_COST_FORMATTED,
      totalItemsCount: COST_ITEMS.length,
      filteredCount: filteredItems.length,
      filteredTotalCost: filteredTotal,
      filteredTotalCostFormatted: `₹${filteredTotal.toLocaleString("en-IN")}`,
      pricingModel: PRICING_MODEL,
      moduleSummaries: MODULE_SUMMARIES,
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
