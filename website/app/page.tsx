import Link from "next/link";
import { TOTAL_COST_FORMATTED } from "./cost-list/data";

export default function Home() {
  return (
    <div className="min-h-screen bg-white text-gray-900 flex flex-col justify-between">
      {/* Top Header */}
      <header className="border-b border-gray-200 bg-white">
        <div className="max-w-5xl mx-auto px-4 sm:px-6 py-4 flex items-center justify-between">
          <div className="flex items-center gap-2">
            <span className="font-bold text-gray-900 text-sm">SIH 2026</span>
            <span className="text-gray-300">/</span>
            <span className="text-sm text-gray-600">Warehouse Robotics Platform</span>
          </div>
          <nav className="flex items-center gap-4 text-xs font-medium text-gray-600">
            <Link href="/cost-list" className="hover:text-gray-900">
              Cost List
            </Link>
            <Link href="/api/cost-list" className="hover:text-gray-900">
              API
            </Link>
          </nav>
        </div>
      </header>

      {/* Main Content */}
      <main className="max-w-5xl mx-auto px-4 sm:px-6 py-12 flex-1 w-full">
        {/* Intro */}
        <div className="mb-8 pb-6 border-b border-gray-200">
          <div className="text-xs font-medium text-gray-500 uppercase tracking-wider mb-1">
            Smart India Hackathon 2026
          </div>
          <h1 className="text-3xl sm:text-4xl font-bold text-gray-900 mb-3">
            Telescopic Mast &amp; Robotic Arm Subsystem
          </h1>
          <p className="text-sm sm:text-base text-gray-600 max-w-3xl leading-relaxed">
            Technical documentation, component specifications, and complete Bill of Materials (BOM) cost breakdown for the automated guided vehicle mast assembly.
          </p>
        </div>

        {/* Feature Cards Grid */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4 mb-8">
          {/* Card 1: Cost List */}
          <Link
            href="/cost-list"
            className="block p-5 border border-gray-200 rounded hover:border-gray-400 bg-white transition"
          >
            <div className="flex items-center justify-between mb-2">
              <span className="text-xs font-mono font-semibold text-gray-600">/cost-list</span>
              <span className="text-xs font-mono font-bold text-gray-900">{TOTAL_COST_FORMATTED}</span>
            </div>
            <h2 className="text-lg font-bold text-gray-900 mb-1">
              Detailed Cost List &amp; BOM →
            </h2>
            <p className="text-xs text-gray-600 leading-relaxed mb-3">
              View the itemized component pricing table across telescopic mast hardware, horizontal expansion slides, BLDC frameless joint servos, and closed-loop steppers.
            </p>
            <span className="text-xs font-medium text-gray-900 underline">
              Open cost list table
            </span>
          </Link>

          {/* Card 2: API */}
          <Link
            href="/api/cost-list"
            className="block p-5 border border-gray-200 rounded hover:border-gray-400 bg-white transition"
          >
            <div className="flex items-center justify-between mb-2">
              <span className="text-xs font-mono font-semibold text-gray-600">/api/cost-list</span>
              <span className="text-xs text-gray-500">JSON &amp; CSV</span>
            </div>
            <h2 className="text-lg font-bold text-gray-900 mb-1">
              Data API Endpoint →
            </h2>
            <p className="text-xs text-gray-600 leading-relaxed mb-3">
              REST endpoint returning raw JSON and CSV format for software integration, procurement pipelines, and automated reporting.
            </p>
            <span className="text-xs font-medium text-gray-900 underline">
              View JSON response
            </span>
          </Link>
        </div>

        {/* Subsystem Specifications Summary */}
        <div className="border border-gray-200 rounded p-5 bg-gray-50 text-xs">
          <h3 className="font-semibold text-gray-900 mb-2">Subsystem Overview</h3>
          <ul className="list-disc list-inside text-gray-700 space-y-1">
            <li><strong>Primary Mast:</strong> 10ft 9-layer aluminum telescopic mast with NEMA 34 closed-loop stepper drive</li>
            <li><strong>Secondary Arm:</strong> 3ft horizontal expansion slide channel with NEMA 23 stepper</li>
            <li><strong>Robotic Joints:</strong> MyActuator RMD series BLDC servos (X8-120 arm pitch, X6-60 wrist pitch, X4-36 wrist roll)</li>
            <li><strong>Controller &amp; Vision:</strong> STM32F407 controller board with industrial Hikrobot camera</li>
          </ul>
        </div>
      </main>

      {/* Footer */}
      <footer className="border-t border-gray-200 py-6 text-center text-xs text-gray-500">
        Smart India Hackathon 2026 • Automated Warehouse Guided Vehicle &amp; Mast Subsystem
      </footer>
    </div>
  );
}
